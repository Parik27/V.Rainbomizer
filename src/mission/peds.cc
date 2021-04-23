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

class CPedFactory;

CPed *(*CPedFactory_CreateNonCopPed_5c6) (CPedFactory *, uint8_t *, uint32_t,
                                          uint64_t, uint8_t);

using namespace NativeLiterals;

class PedRandomizer
{
    inline static std::mutex            CreatePedMutex;
    inline static std::vector<uint32_t> m_NsfwModels;
    inline static bool                  bSkipNextPedRandomization = false;

    inline static std::map<CPed *, std::pair<uint32_t, uint32_t>>
        sm_PedModelFixupMap;

    static auto &
    Config ()
    {
        static struct Config
        {
            std::string ForcedPed        = "";
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

#define SWAP_FIELD(field) std::swap (modelA->field, modelB->field);

            SWAP_FIELD (m_nCombatInfo);
            SWAP_FIELD (m_nPedType);
            SWAP_FIELD (GetInitInfo ().m_nTaskDataName);
            SWAP_FIELD (GetInitInfo ().m_nRelationshipGroup);
            SWAP_FIELD (GetInitInfo ().m_nDecisionMakerName);
            SWAP_FIELD (GetInitInfo ().m_nNavCapabilitiesName);
            SWAP_FIELD (GetInitInfo ().m_nMotionTaskDataSetName);
            SWAP_FIELD (GetInitInfo ().m_nDefaultTaskDataSetName);
            SWAP_FIELD (m_nMovementClipSet);

#undef SWAP_FIELD
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
        sm_PedModelFixupMap[ped] = std::make_pair (newModel, model);

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

        auto *model = ped->m_pModelInfo;
        if (auto *data = LookupMap (sm_PedModelFixupMap, ped))
            {
                auto *storedModel = CStreaming::GetModelByIndex (data->first);
                auto *intendedModel
                    = CStreaming::GetModelByIndex (data->second);

                if (storedModel->m_nHash == model->m_nHash)
                    model = intendedModel;
            }

        return model->m_nHash;
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

public:
    /*******************************************************/
    PedRandomizer ()
    {
        std::string ForcedPed;
        if (!ConfigManager::ReadConfig (
                "PedRandomizer", std::pair ("ForcedPed", &Config ().ForcedPed),
                std::pair ("RandomizePlayer", &Config ().RandomizePlayer)))
            return;

        if (Config ().ForcedPed.size ())
            Config ().ForcedPedHash = rage::atStringHash (Config ().ForcedPed);

        InitialiseAllComponents ();

        // Hooks
        RegisterHook ("8b c0 ? 8b ? ? 8b ? ? 88 7c ? ? e8 ? ? ? ? eb ?", 13,
                      CPedFactory_CreateNonCopPed_5c6,
                      RandomizePed<CPedFactory_CreateNonCopPed_5c6>);

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

        // Fix for scripts where a certain hash is required for a certain ped.
        "GET_ENTITY_MODEL"_n.Hook (FixupScriptEntityModel);

        // This hook fixes a crash in CTaskUseScenario, where the ped bounds
        // type and include flags are accessed; In case of fish bounds, these
        // type and include flags are not allocated causing a crash.
        REGISTER_HOOK ("0f 29 40 30 e8 ? ? ? ? b2 01 ? 8b cf e8", 4,
                       AllocateTypeAndIncludeFlagsForFishBounds, void,
                       phBoundComposite *, bool, bool);
    }
} peds;
