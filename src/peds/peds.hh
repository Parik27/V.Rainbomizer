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

class PedRandomizer_PedHashes
{
    std::string m_ForcedPedStr;
    std::vector<uint32_t> m_ForcedPedHashes;

public:
    /*******************************************************/
    PedRandomizer_PedHashes () = default;

    /*******************************************************/
    PedRandomizer_PedHashes (const std::string &forcedPedStr)
        : m_ForcedPedStr (forcedPedStr)
    {
        UpdateHashes (forcedPedStr);
    }

    /*******************************************************/
    void
    operator= (const std::string &forcedPedStr)
    {
        if (m_ForcedPedStr != forcedPedStr)
        {
            m_ForcedPedStr = forcedPedStr;
            UpdateHashes (forcedPedStr);
        }
    }

    /*******************************************************/
    void
    UpdateHashes (std::string forcedPedStr)
    {
        m_ForcedPedHashes.clear ();

        if (forcedPedStr.size ())
            {
                while (true)
                    {
                        auto splitPos = forcedPedStr.find (',');
                        auto forcedPed = std::string (forcedPedStr.substr (0, splitPos));

                        // trimming
                        forcedPed.erase (0, forcedPed.find_first_not_of (' '));
                        if (forcedPed.find_last_not_of (' ') != forcedPed.npos)
                            forcedPed.erase (forcedPed.find_last_not_of (' ') + 1);
                        
                        if (forcedPed.size ())
                            m_ForcedPedHashes
                                .push_back (rage::atStringHash (forcedPed));
                        
                        if (splitPos == forcedPedStr.npos)
                            {
                                break;
                            }
                        
                        forcedPedStr.erase (0, splitPos + 1);
                    }
            }
    }

    /*******************************************************/
    const std::vector<uint32_t> &
    Get () const
    {
        return m_ForcedPedHashes;
    }
};

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
            PedRandomizer_PedHashes ForcedPedHashes;
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
