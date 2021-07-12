#include <cstdint>
#include <mutex>
#include <utility>

#include <common/config.hh>
#include <common/parser.hh>

#include <scrThread.hh>

#include "CModelInfo.hh"
#include "common/logger.hh"
#include "peds_Compatibility.hh"
#include "peds_Streaming.hh"
#include "peds_PlayerFixes.hh"
#include "peds_Swapper.hh"

class CPedFactory;

CPed *(*CPedFactory_CreateNonCopPed_5c6) (CPedFactory *, uint8_t *, uint32_t,
                                          uint64_t, uint8_t);

class PedRandomizer
{
    inline static std::mutex CreatePedMutex;
    inline static bool       bSkipNextPedRandomization = false;

    constexpr static const char PedsFileName[] = "CutsceneModelsPeds.txt";
    using CutsPedsRandomizer
        = DataFileBasedModelRandomizer<PedsFileName,
                                       CStreaming::GetModelByHash<>>;

    using ModelSwapper = PedRandomizer_ModelSwapper;

    static auto &
    Config ()
    {
        static struct Config
        {
            std::string ForcedPed               = "";
            std::string ForcedClipset           = "";
            uint32_t    ForcedPedHash           = -1;
            bool        EnableNSFWModels        = false;
            bool        RandomizePlayer         = true;
            bool        RandomizePeds           = true;
            bool        RandomizeSpecialAbility = true;
            bool        IncludeUnusedAbilities  = false;
            bool        UseCutsceneModelsFile   = true;
        } m_Config;

        return m_Config;
    }

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
    static bool
    ShouldRandomizePedModel (uint32_t model)
    {
        if (IsPlayerModel (CStreaming::GetModelByIndex (model)))
            {
                if (!Config ().RandomizePlayer)
                    return false;
            }
        else if (!Config ().RandomizePeds)
            return false;

        if (PedRandomizer_Streaming::IsPedBlacklisted (model))
            return false;

        return true;
    }

    /*******************************************************/
    static uint32_t
    GetRandomPedModel (uint32_t model)
    {
        if (!ShouldRandomizePedModel (model))
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
        uint32_t randomPed = PedRandomizer_Streaming::GetRandomLoadedPed (
            Config ().EnableNSFWModels);

        if (randomPed != -1)
            return randomPed;

        return model;
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
        PedRandomizer_Streaming::Process ();

        uint32_t           newModel = GetRandomPedModel (model);
        const ModelSwapper swap (newModel, model);
        swap.OverrideClipset (Config ().ForcedClipset);

        PedRandomizerCompatibility::SetRandomizingPed (
            CStreaming::GetModelByIndex<CPedModelInfo> (model));

        CPed *ped = CPedFactory__CreatePed (fac, p2, newModel, p4, p5);

        PedRandomizerCompatibility::SetRandomizingPed (nullptr);
        PedRandomizerCompatibility::AddRandomizedPed (ped, model, newModel);
        PedRandomizer_PlayerFixes::UpdatePlayerHash ();
        PedRandomizer_PlayerFixes::SetSpecialAbility (
            ped, Config ().RandomizeSpecialAbility,
            Config ().IncludeUnusedAbilities);

        return ped;
    }

    /*******************************************************/
    template <auto &CCutsceneAnimatedActorEntity__CreatePed>
    static void
    RandomizeCutscenePeds (class CCutsceneAnimatedActorEntity *entity,
                           uint32_t model, bool p3)
    {
        static CutsPedsRandomizer sm_Randomizer;

        // Randomize model (remember model here is the idx, so convert to hash)
        uint32_t hash = CStreaming::GetModelHash (model);

        if (!sm_Randomizer.RandomizeObject (hash))
            return CCutsceneAnimatedActorEntity__CreatePed (entity, model, p3);

        // Load the new model
        uint32_t newModel = CStreaming::GetModelIndex (hash);
        if (!CStreaming::HasModelLoaded (newModel))
            {
                CStreaming::RequestModel (newModel, 0);
                CStreaming::LoadAllObjects (false);
            }

        if (CStreaming::HasModelLoaded (newModel))
            {
                model                     = newModel;
                bSkipNextPedRandomization = true;
            }

        // Spawn the ped
        CCutsceneAnimatedActorEntity__CreatePed (entity, model, p3);
    }

public:
    /*******************************************************/
    PedRandomizer ()
    {
        std::string ForcedPed;
        if (!ConfigManager::ReadConfig (
                "PedRandomizer", std::pair ("ForcedPed", &Config ().ForcedPed),
                std::pair ("RandomizePlayer", &Config ().RandomizePlayer),
                std::pair ("RandomizePeds", &Config ().RandomizePeds),
                std::pair ("UseCutsceneModelsFile",
                           &Config ().UseCutsceneModelsFile),
                std::pair ("ForcedClipset", &Config ().ForcedClipset),
                std::pair ("RandomizeSpecialAbility",
                           &Config ().RandomizeSpecialAbility),
                std::pair ("IncludeUnusedAbilities",
                           &Config ().IncludeUnusedAbilities)))
            return;

        if (Config ().ForcedPed.size ())
            Config ().ForcedPedHash = rage::atStringHash (Config ().ForcedPed);

        InitialiseAllComponents ();

        // Hooks
        RegisterHook ("8b c0 ? 8b ? ? 8b ? ? 88 7c ? ? e8 ? ? ? ? eb ?", 13,
                      CPedFactory_CreateNonCopPed_5c6,
                      RandomizePed<CPedFactory_CreateNonCopPed_5c6>);

        if (Config ().UseCutsceneModelsFile)
            REGISTER_HOOK ("85 ff 75 ? 45 8a c4 e8 ? ? ? ? 45 84 e4 74", 7,
                           RandomizeCutscenePeds, void,
                           class CCutsceneAnimatedActorEntity *, uint32_t,
                           bool);

        REGISTER_HOOK (
            "88 44 ? ? 40 88 7c ? ? e8 ? ? ? ? ? 8b d8 ? 85 c0 0f 84", 9,
            RandomizePed, CPed *, CPedFactory *, uint8_t *, uint32_t, uint64_t,
            uint8_t);

        PedRandomizer_Streaming::Initialise ();
        PedRandomizer_PlayerFixes::Initialise ();
    }
} peds;
