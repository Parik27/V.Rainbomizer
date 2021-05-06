#include <cstdint>
#include <mutex>
#include <utility>

#include <common/config.hh>

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

    using ModelSwapper = PedRandomizer_ModelSwapper;

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

        CPed *ped = CPedFactory__CreatePed (fac, p2, newModel, p4, p5);

        PedRandomizerCompatibility::AddRandomizedPed (ped, model, newModel);
        PedRandomizer_PlayerFixes::UpdatePlayerHash ();
        PedRandomizer_PlayerFixes::RandomizeSpecialAbility (ped);

        return ped;
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

        PedRandomizer_Streaming::Initialise ();
        PedRandomizer_PlayerFixes::Initialise ();
    }
} peds;
