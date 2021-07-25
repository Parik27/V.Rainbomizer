#pragma once

#include "missions_Order.hh"
#include "missions_Globals.hh"
#include "missions_PlayerSwitch.hh"
#include "missions_Flow.hh"
#include "missions_Data.hh"
#include "missions_Code.hh"
#include "missions_Cmds.hh"

#include "common/common.hh"
#include <cstdint>
#include <vector>

class MissionRandomizer_Components
{
    inline static std::vector<uint32_t> sm_ScriptsAllowedInPause;

    template <auto &...Components> class ComponentSet
    {
    public:
        static bool
        Process (scrProgram *program, scrThreadContext *ctx)
        {
            bool ret = true;
            (..., (ret = Components.Process (program, ctx) ? ret : false));

            return ret;
        }
    };

public:
    inline static MissionRandomizer_Data sm_Data{
        Rainbomizer::Common::GetRainbomizerDataFile ("Missions.txt")};

    inline static MissionRandomizer_GlobalsManager sm_Globals;
    inline static MissionRandomizer_OrderManager   sm_Order;
    inline static MissionRandomizer_PlayerSwitcher sm_PlayerSwitcher;
    inline static MissionRandomizer_Flow           sm_Flow;
    inline static MissionRandomizer_CodeFixes      sm_CodeFixes;
    inline static MissionRandomizer_Commands       sm_Cmds;

    using ComponentList = ComponentSet<sm_Globals, sm_Order, sm_Flow,
                                       sm_PlayerSwitcher, sm_Cmds>;

    static bool
    Process (scrProgram *program, scrThreadContext *ctx)
    {
        return ComponentList::Process (program, ctx);
    }

    static bool
    IsScriptAllowedOnPause (uint32_t name)
    {
        bool allowed = DoesElementExist (sm_ScriptsAllowedInPause, name);
        sm_ScriptsAllowedInPause.clear ();

        return allowed;
    }

    static void
    AllowScriptOnPauseThisFrame (uint32_t name)
    {
        sm_ScriptsAllowedInPause.push_back (name);
    }

    static auto &
    Config ()
    {
        static struct Config
        {
            std::string Seed;
            bool        ForceSeedOnSaves;
            bool        LogMissionOrder;
            bool        EnableFastSkips;

            std::string ForcedMission;

        } m_Config;

        return m_Config;
    }
};
