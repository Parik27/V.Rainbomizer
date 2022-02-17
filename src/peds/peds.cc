#include "peds.hh"
#include "clothes_Queue.hh"

#include <cstdint>
#include <mutex>
#include <utility>

#include <common/config.hh>
#include <common/parser.hh>
#include <common/mods.hh>

#include <scrThread.hh>
#include <Random.hh>

#include "CModelInfo.hh"
#include "Utils.hh"
#include "common/logger.hh"
#include "peds/clothes_Queue.hh"

#include <CStreaming.hh>

class CPedFactory;

CPed *(*CPedFactory_CreateNonCopPed_5c6) (CPedFactory *, uint8_t *, uint32_t,
                                          uint64_t, uint8_t);

class PedRandomizer
{
    using PR = PedRandomizer_Components;

    inline static std::mutex CreatePedMutex;

    constexpr static const char PedsFileName[] = "CutsceneModelsPeds.txt";
    using CutsPedsRandomizer
        = DataFileBasedModelRandomizer<PedsFileName,
                                       CStreaming::GetModelByHash<>>;

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
        if (!ModCompatibility::ShouldRandomize ())
            return false;

        if (IsPlayerModel (CStreaming::GetModelByIndex (model)))
            {
                if (!PR::Config ().RandomizePlayer)
                    return false;
            }
        else if (!PR::Config ().RandomizePeds)
            return false;

        if (PedRandomizer_Streaming::IsPedBlacklisted (model))
            return false;

        return true;
    }

    /*******************************************************/
    static uint32_t
    GetRandomPedModel (uint32_t model)
    {
        if (PR::nForcedModelNextRandomization != -1u)
            return std::exchange (PR::nForcedModelNextRandomization, -1u);

        if (!ShouldRandomizePedModel (model))
            return model;

        // Forced Ped
        if (!PR::Config ().ForcedPedHashes.empty ())
            {
                uint32_t id = CStreaming::GetModelIndex (
                    PR::Config ().ForcedPedHashes [ 0, RandomInt(PR::Config()
                    .ForcedPedHashes.size() - 1) ]);

                if (CStreaming::HasModelLoaded (id))
                    return id;

                CStreaming::RequestModel (id, 0);
                CStreaming::LoadAllObjects (false);

                if (!CStreaming::HasModelLoaded (id))
                    return model;

                return id;
            }

        // Random Ped
        uint32_t randomPed = PedRandomizer_Streaming::GetRandomLoadedPed (
            PR::Config ().EnableNSFWModels,
            RandomInt (100) < PR::Config ().OddsOfPlayerModels);

        if (randomPed != -1u)
            return randomPed;

        return model;
    }

    /*******************************************************/
    template <auto &CPedFactory__CreatePed>
    static CPed *
    RandomizePed (CPedFactory *fac, uint8_t *p2, uint32_t model, uint64_t p4,
                  uint8_t p5)
    {
        if (std::exchange (PR::bSkipNextPedRandomization, false))
            return CPedFactory__CreatePed (fac, p2, model, p4, p5);

        const std::lock_guard g (CreatePedMutex);
        PedRandomizer_Streaming::Process ();

        uint32_t          newModel = GetRandomPedModel (model);
        const PR::Swapper swap (newModel, model);
        swap.OverrideClipset (PR::Config ().ForcedClipset);

        PedRandomizer_Compatibility::SetRandomizingPed (
            CStreaming::GetModelByIndex<CPedModelInfo> (model));

        CPed *ped = CPedFactory__CreatePed (fac, p2, newModel, p4, p5);

        PR::Compatibility::SetRandomizingPed (nullptr);
        PR::Compatibility::AddRandomizedPed (ped, model, newModel);
        PR::PlayerFixes::UpdatePlayerHash ();
        PR::PlayerFixes::SetSpecialAbility (
            ped, PR::Config ().RandomizeSpecialAbility,
            PR::Config ().IncludeUnusedAbilities);

        ClothesRandomizer_Queue::Add (ped);

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

        if (!PR::Config ().RandomizeCutscenePeds
            || !ShouldRandomizePedModel (model)
            || !sm_Randomizer.RandomizeObject (hash))
            return CCutsceneAnimatedActorEntity__CreatePed (entity, model, p3);

        // Load the new model
        uint32_t newModel = CStreaming::GetModelIndex (hash);
        if (!CStreaming::HasModelLoaded (newModel))
            {
                CStreaming::RequestModel (newModel, 0);
                CStreaming::LoadAllObjects (false);
            }

        if (CStreaming::HasModelLoaded (newModel))
            PR::nForcedModelNextRandomization = newModel;

        // Spawn the ped
        CCutsceneAnimatedActorEntity__CreatePed (entity, model, p3);
    }

public:
    /*******************************************************/
    PedRandomizer ()
    {
#define OPTION(option) std::pair (#option, &PR::Config ().option)

        if (!ConfigManager::ReadConfig (
                "PedRandomizer", OPTION (ForcedPed), OPTION (RandomizePlayer),
                OPTION (RandomizeCutscenePeds), OPTION (UseCutsceneModelsFile),
                OPTION (ForcedClipset), OPTION (RandomizeSpecialAbility),
                OPTION (IncludeUnusedAbilities),
                OPTION (EnableAnimalMotionFixes), OPTION (EnableAnimalFixes),
                OPTION (EnablePlayerFixes), OPTION (EnableMainFixes),
                OPTION (EnableNoLowBudget), OPTION (EnableBlipsAlwaysVisible),
                OPTION (OddsOfPlayerModels)))
            return;

        if (PR::Config ().ForcedPed.size ())
            {
                std::string forcedPeds = PR::Config ().ForcedPed;

                while (true)
                    {
                        size_t splitPos = forcedPeds.find (',');

                        std::string forcedPed = forcedPeds.substr (0, splitPos);

                        // trimming
                        forcedPed.erase (0, forcedPed.find_first_not_of (' '));
                        if (forcedPed.find_last_not_of (' ') != forcedPed.npos)
                            forcedPed.erase (forcedPed.find_last_not_of (' ') + 1);
                        
                        // just in case it was just whitespace
                        if (forcedPed.size ())
                            PR::Config ().ForcedPedHashes
                                .push_back (rage::atStringHash (forcedPed));
                        
                        if (splitPos == forcedPeds.npos)
                            {
                                break;
                            }
                        
                        forcedPeds.erase (0, splitPos + 1);
                    }
            }

        InitialiseAllComponents ();

        // Hooks
        RegisterHook ("8b c0 ? 8b ? ? 8b ? ? 88 7c ? ? e8 ? ? ? ? eb ?", 13,
                      CPedFactory_CreateNonCopPed_5c6,
                      RandomizePed<CPedFactory_CreateNonCopPed_5c6>);

        if (PR::Config ().UseCutsceneModelsFile)
            REGISTER_HOOK ("85 ff 75 ? 45 8a c4 e8 ? ? ? ? 45 84 e4 74", 7,
                           RandomizeCutscenePeds, void,
                           class CCutsceneAnimatedActorEntity *, uint32_t,
                           bool);

        REGISTER_HOOK (
            "88 44 ? ? 40 88 7c ? ? e8 ? ? ? ? ? 8b d8 ? 85 c0 0f 84", 9,
            RandomizePed, CPed *, CPedFactory *, uint8_t *, uint32_t, uint64_t,
            uint8_t);

        PR::Initialise ();
    }
} peds;
