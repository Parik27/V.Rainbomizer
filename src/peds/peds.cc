#include "CModelInfo.hh"
#include "Patterns/Patterns.hh"
#include "Utils.hh"
#include "CPed.hh"
#include "CStreaming.hh"
#include "CTheScripts.hh"
#include "CutSceneManager.hh"

#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <stdio.h>
#include <type_traits>
#include <utility>
#include <vector>

#include "common/common.hh"
#include "common/config.hh"
#include "common/logger.hh"
#include "injector/hooking.hpp"
#include "phBound.hh"
#include "rage.hh"
#include "fwExtensibleBase.hh"
#include "scrThread.hh"

#include "peds_Compatibility.hh"

class CPedFactory;

CPed *(*CPedFactory_CreateNonCopPed_5c6) (CPedFactory *, uint8_t *, uint32_t,
                                          uint64_t, uint8_t);

using namespace NativeLiterals;

class PedRandomizer
{
    inline static std::mutex            CreatePedMutex;
    inline static std::vector<uint32_t> m_NsfwModels;
    inline static bool                  bSkipNextPedRandomization = false;

    static auto &
    Config ()
    {
        static struct Config
        {
            std::string ForcedPed        = "";
            std::string ForcedClipset    = "";
            uint32_t    ForcedPedHash    = -1;
            bool        EnableNSFWModels = false;
            bool        RandomizePlayer  = true;
        } m_Config;

        return m_Config;
    }

    /*******************************************************/
    /* RAII wrapper to swap two models */
    /*******************************************************/
    class ModelSwapper
    {
        uint32_t m_IdA;
        uint32_t m_IdB;

        static void
        SwapModels (uint32_t a, uint32_t b)
        {

            /*
              Most Important:

              CombatInfo
              NavCapabilities
              TaskDataInfo

              Others:
              RelationshipGroup?
              PedType?
              DecisionMaker??
              DefaultUnarmedWeapon??
              PedPersonality???
              PedCapsule???

              Decided Against:

              PerceptionInfo
              BrawlingStyle
              PedComponentCloth
              PedComponentSet
              DefaultTaskData
             */

            auto modelA = CStreaming::GetModelByIndex<CPedModelInfo> (a);
            auto modelB = CStreaming::GetModelByIndex<CPedModelInfo> (b);

            CPedModelInfo *clipsetSrc = nullptr;
            if (!Config ().ForcedClipset.empty ())
                {
                    clipsetSrc = CStreaming::GetModelByHash<CPedModelInfo> (
                        rage::atStringHash (Config ().ForcedClipset));
                }

#define SWAP_FIELD(field) std::swap (modelA->field, modelB->field);
#define SWAP_FIELD_CLIP(field) modelA->field = clipsetSrc->field;

            SWAP_FIELD (m_nCombatInfo);
            SWAP_FIELD (m_nPedType);
            SWAP_FIELD (m_nPersonality);

            SWAP_FIELD (GetInitInfo ().m_nTaskDataName);
            SWAP_FIELD (GetInitInfo ().m_nRelationshipGroup);
            SWAP_FIELD (GetInitInfo ().m_nDecisionMakerName);
            SWAP_FIELD (GetInitInfo ().m_nNavCapabilitiesName);

#ifdef FLYING_BIRDS
            SWAP_FIELD (m_nMovementClipSet);
            SWAP_FIELD (GetInitInfo ().m_nMotionTaskDataSetName);
#endif

            if (clipsetSrc)
                {
                    SWAP_FIELD_CLIP (m_nMovementClipSet);
                    SWAP_FIELD_CLIP (GetInitInfo ().m_nMotionTaskDataSetName);
                }

#undef SWAP_FIELD
#undef SET_FIELD
        }

    public:
        ModelSwapper (uint32_t idA, uint32_t idB) : m_IdA (idA), m_IdB (idB)
        {
            SwapModels (idA, idB);
        }

        ~ModelSwapper () { SwapModels (m_IdA, m_IdB); }
    };

    /*******************************************************/
    static bool
    IsPlayerModel (CBaseModelInfo *model)
    {
        switch (model->m_nHash)
            {
            case "player_zero"_joaat:
            case "player_one"_joaat:
            case "player_two"_joaat: return true;
            }

        return false;
    }

    /*******************************************************/
    static void
    ProcessPedStreaming ()
    {
        const int     TIMEOUT   = 5;
        static size_t timestamp = time (NULL);

        if (time (NULL) - timestamp > TIMEOUT)
            {
                auto PedHashes = Rainbomizer::Common::GetPedHashes ();
                CStreaming::RequestModel (CStreaming::GetModelIndex (
                                              GetRandomElement (PedHashes)),
                                          0);

                timestamp = time (NULL);
            }
    }

    /*******************************************************/
    static bool
    ReadNsfwModelsList ()
    {
        FILE *f
            = Rainbomizer::Common::GetRainbomizerDataFile ("NSFW_Models.txt");

        if (!f)
            return false;

        char line[256] = {0};
        while (fgets (line, 256, f))
            m_NsfwModels.push_back (rage::atStringHash (line));

        return true;
    }

    /*******************************************************/
    static void
    RemoveNsfwModels (std::set<uint32_t> &set)
    {
        static bool hasNsfwModelsList = ReadNsfwModelsList ();
        if (hasNsfwModelsList)
            {
                for (auto i : m_NsfwModels)
                    set.erase (i);
            }
    }

    /*******************************************************/
    static uint32_t
    GetRandomPedModel (uint32_t model)
    {
        if (!Config ().RandomizePlayer
            && IsPlayerModel (CStreaming::GetModelByIndex (model)))
            return model;

        // Forced Ped
        if (!Config ().ForcedPed.empty ())
            {
                uint32_t id = CStreaming::GetModelIndex (
                    rage::atStringHash (Config ().ForcedPed));

                if (CStreaming::HasModelLoaded (id))
                    return id;

                CStreaming::RequestModel (id, 0);
                return model;
            }

        // Random Ped
        std::set<uint32_t> peds;
        auto               groups = CStreaming::sm_Instance;

        groups->mAppropriatePedsSet.for_each (
            [&peds] (int val) { peds.insert (val); });
        groups->mInAppropriatePedsSet.for_each (
            [&peds] (int val) { peds.insert (val); });
        groups->mCopsSet.for_each ([&peds] (int val) { peds.insert (val); });

        if (!Config ().EnableNSFWModels)
            RemoveNsfwModels (peds);

        if (peds.size () < 1)
            return model;

        return GetRandomElement (peds);
    }

    /*******************************************************/
    template <auto &CPedFactory__CreatePed>
    static CPed *
    RandomizePed (CPedFactory *fac, uint8_t *p2, uint32_t model, uint64_t p4,
                  uint8_t p5)
    {
        if (std::exchange (bSkipNextPedRandomization, false))
            return CPedFactory__CreatePed (fac, p2, model, p4, p5);

        const std::lock_guard g (CreatePedMutex);

        ProcessPedStreaming ();

        uint32_t           newModel = GetRandomPedModel (model);
        const ModelSwapper swap (newModel, model);

        CPed *ped = CPedFactory__CreatePed (fac, p2, newModel, p4, p5);
        PedRandomizerCompatibility::AddRandomizedPed (ped, model, newModel);

        return ped;
    }

    /*******************************************************/
    template <auto &CCutsceneAnimatedActorEntity__CreatePed>
    static void
    SkipRandomizingCutscenePeds (class CCutsceneAnimatedActorEntity *entity,
                                 uint32_t model, bool p3)
    {
        bSkipNextPedRandomization = true;
        CCutsceneAnimatedActorEntity__CreatePed (entity, model, p3);
    }

    /*******************************************************/
    static uint32_t
    FixupScriptEntityModel (uint32_t guid)
    {
        CPed *ped = static_cast<CPed *> (fwScriptGuid::GetBaseFromGuid (guid));

        if (!ped || !ped->m_pModelInfo)
            return 0;

        return PedRandomizerCompatibility::GetOriginalModel (ped)->m_nHash;
    }

    /*******************************************************/
    static void
    FixupScriptEntityModel (scrThread::Info *info)
    {
        info->GetReturn () = FixupScriptEntityModel (info->GetArg (0));
    }

    /*******************************************************/
    template <auto &phBoundComposite_CalculateExtents>
    static void
    AllocateTypeAndIncludeFlagsForFishBounds (phBoundComposite *bound, bool p1,
                                              bool p2)
    {
        if (!bound->OwnedTypeAndIncludeFlags)
            {
                bound->AllocateTypeAndIncludeFlags ();
                bound->TypeAndIncludeFlags[1] &= 3221225471;
            }

        phBoundComposite_CalculateExtents (bound, p1, p2);
    }

    /*******************************************************/
    template <auto &CutSceneManager_InitPlayerObj>
    static void
    CorrectPlayerObjIdx (CutSceneManager *mgr)
    {
        CPed *          player = CPedFactory::Get ()->pPlayer;
        CBaseModelInfo *model  = PedRandomizerCompatibility::GetOriginalModel (
            CPedFactory::Get ()->pPlayer);

        // Restore original model for this function and change it back after
        // done.
        std::swap (player->m_pModelInfo, model);
        CutSceneManager_InitPlayerObj (mgr);
        std::swap (player->m_pModelInfo, model);
    }

    /*******************************************************/
    template <auto &CutSceneManager_RegisterEntity>
    static void
    CorrectRegisterEntity (CutSceneManager *mgr, CEntity *entity,
                           uint32_t *handle, uint32_t *modelHash, bool p5,
                           bool p6, bool p7, uint32_t p8)
    {
        if (entity
            && entity->m_pModelInfo->GetType ()
                   == eModelInfoType::MODEL_INFO_PED)
            {
                *modelHash = PedRandomizerCompatibility::GetOriginalModel (
                                 static_cast<CPed *> (entity))
                                 ->m_nHash;
            }

        CutSceneManager_RegisterEntity (mgr, entity, handle, modelHash, p5, p6,
                                        p7, p8);
    }

    /*******************************************************/
    template <auto &CStreaming_GetRandomPedToLoad>
    static uint32_t
    RandomizePedToLoad (CStreaming *str, bool p2)
    {
        // CStreaming_GetRandomPedToLoad (str, p2);
        auto &peds = Rainbomizer::Common::GetPedHashes ();

        for (int i = 0; i < 16; i++)
            {
                uint32_t vehicle = GetRandomElement (peds);
                uint32_t pedIndex;
                auto model = CStreaming::GetModelAndIndexByHash<CPedModelInfo> (
                    vehicle, pedIndex);

                if (CStreaming::HasModelLoaded (pedIndex))
                    continue;

                return pedIndex;
            }
        return 65535; // -1/uint16_t
    }

public:
    /*******************************************************/
    PedRandomizer ()
    {
        std::string ForcedPed;
        if (!ConfigManager::ReadConfig (
                "PedRandomizer", std::pair ("ForcedPed", &Config ().ForcedPed),
                std::pair ("RandomizePlayer", &Config ().RandomizePlayer),
                std::pair ("ForcedClipset", &Config ().ForcedClipset)))
            return;

        if (Config ().ForcedPed.size ())
            Config ().ForcedPedHash = rage::atStringHash (Config ().ForcedPed);

        InitialiseAllComponents ();

        // Hooks
        RegisterHook ("8b c0 ? 8b ? ? 8b ? ? 88 7c ? ? e8 ? ? ? ? eb ?", 13,
                      CPedFactory_CreateNonCopPed_5c6,
                      RandomizePed<CPedFactory_CreateNonCopPed_5c6>);

        REGISTER_HOOK (
            "88 44 ? ? 40 88 7c ? ? e8 ? ? ? ? ? 8b d8 ? 85 c0 0f 84", 9,
            RandomizePed, CPed *, CPedFactory *, uint8_t *, uint32_t, uint64_t,
            uint8_t);

        // We don't want to randomize cutscene peds since those are managed by
        // the cutscene randomizer.
#if 0
        REGISTER_HOOK ("85 ff 75 ? 45 8a c4 e8 ? ? ? ? 45 84 e4 74", 7,
                       SkipRandomizingCutscenePeds, void,
                       class CCutsceneAnimatedActorEntity *, uint32_t, bool);
#endif

        // This patch fixes CTaskExitVehicle from crashing the game if the ped
        // exiting the vehicle doesn't have a helmet.
        injector::MakeNOP (
            hook::get_pattern ("44 38 70 0a 74 ? 40 84 f6 75 ? b9 ? ? ? ? e8",
                               4),
            2);

        // This patch enables phone models for all ped models (for player
        // randomizer)
        injector::MakeNOP (
            hook::get_pattern (
                "8b ? ? ? ? ? 44 38 70 09 74 ? 40 84 f6 75 ? b9 30 00 00 00 ",
                10),
            2);

        // Fix for scripts where a certain hash is required for a certain ped.
        "GET_ENTITY_MODEL"_n.Hook (FixupScriptEntityModel);

        // This hook fixes a crash in CTaskUseScenario, where the ped bounds
        // type and include flags are accessed; In case of fish bounds, these
        // type and include flags are not allocated causing a crash.
        REGISTER_HOOK ("0f 29 40 30 e8 ? ? ? ? b2 01 ? 8b cf e8", 4,
                       AllocateTypeAndIncludeFlagsForFishBounds, void,
                       phBoundComposite *, bool, bool);

        // Hook for cutscenes to properly get the object model to register for
        // the cutscene.
        REGISTER_HOOK ("8b d9 40 38 3d ? ? ? ? 75 ? e8", 11,
                       CorrectPlayerObjIdx, void, CutSceneManager *);

        REGISTER_HOOK (
            "83 ec 20 80 3d ? ? ? ? 00 ? 8b f1 74 ? e8 ? ? ? ? eb ? e8", 22,
            RandomizePedToLoad, uint32_t, CStreaming *, bool);

        REGISTER_HOOK (
            "c6 44 ? ? 00 ? 8d ? f0 ? 8d ? f4 ? 8b c8 e8 ? ? ? ? ? 8b 5c", 16,
            CorrectRegisterEntity, void, CutSceneManager *, CEntity *,
            uint32_t *, uint32_t *, bool, bool, bool, uint32_t);

        // Hook to prevent peds with flag (DiesOnRagdoll) from dying
        injector::MakeNOP (
            hook::get_pattern (
                "75 ? ? 8b ? ? ? ? ? ? 85 c0 74 ? 44 38 60 0f 75", 18),
            2);
    }
} peds;
