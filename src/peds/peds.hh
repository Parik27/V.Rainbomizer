#pragma once

#include <string>
#include <vector>

#include "peds_Compatibility.hh"
#include "peds_Streaming.hh"
#include "peds_PlayerFixes.hh"
#include "peds_Swapper.hh"
#include "peds_AnimalMotion.hh"
#include "peds_AnimalFixes.hh"
#include "peds_MainFixes.hh"

class PedRandomizer_Components
{
public:
    static auto &
    Config ()
    {
        static struct Config
        {
            std::string ForcedPed                = "";
            std::string ForcedClipset            = "";
            std::vector<uint32_t> ForcedPedHashes;
            bool        EnableNSFWModels         = false;
            bool        RandomizePlayer          = true;
            bool        RandomizePeds            = true;
            bool        RandomizeCutscenePeds    = true;
            bool        RandomizeSpecialAbility  = true;
            bool        IncludeUnusedAbilities   = false;
            bool        UseCutsceneModelsFile    = true;
            bool        EnableAnimalMotionFixes  = true;
            bool        EnableAnimalFixes        = true;
            bool        EnablePlayerFixes        = true;
            bool        EnableMainFixes          = true;
            bool        EnableNoLowBudget        = true;
            bool        EnableBlipsAlwaysVisible = true;
            int         OddsOfPlayerModels       = 20;
        } m_Config;

        return m_Config;
    }

    inline static bool     bSkipNextPedRandomization     = false;
    inline static uint32_t nForcedModelNextRandomization = -1u;

    using PlayerFixes   = PedRandomizer_PlayerFixes;
    using Swapper       = PedRandomizer_ModelSwapper;
    using Streaming     = PedRandomizer_Streaming;
    using Compatibility = PedRandomizer_Compatibility;
    using AnimalMotion  = PedRandomizer_AnimalMotion;
    using AnimalFixes   = PedRandomizer_AnimalFixes;
    using MainFixes     = PedRandomizer_MainFixes;

    static void
    Initialise ()
    {
        Streaming::Initialise ();

        if (Config ().EnablePlayerFixes)
            PlayerFixes::Initialise ();

        if (Config ().EnableAnimalMotionFixes)
            AnimalMotion::Initialise ();

        if (Config ().EnableAnimalFixes)
            AnimalFixes::Initialise ();

        if (Config ().EnableMainFixes)
            MainFixes::Initialise ();
    }
};
