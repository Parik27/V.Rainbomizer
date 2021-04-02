#pragma once

#include "missions_Order.hh"
#include "missions_Globals.hh"
#include "missions_PlayerSwitch.hh"
#include "missions_Flow.hh"
#include "missions_Data.hh"
#include "missions_Code.hh"
#include "missions_Cmds.hh"

#include "common/common.hh"

class MissionRandomizer_Components
{
    template <auto &...Components>
    static bool
    ProcessComponents (scrProgram *program, scrThreadContext *ctx)
    {
        bool ret = true;
        (..., (ret = Components.Process (program, ctx) ? ret : false));

        return ret;
    }

public:
    inline static MissionRandomizer_Data sm_Data{
        Rainbomizer::Common::GetRainbomizerDataFile ("Missions.txt")};

    inline static MissionRandomizer_GlobalsManager sm_Globals;
    inline static MissionRandomizer_OrderManager   sm_Order;
    inline static MissionRandomizer_PlayerSwitcher sm_PlayerSwitcher;
    inline static MissionRandomizer_Flow           sm_Flow;
    inline static MissionRandomizer_CodeFixes      sm_CodeFixes;
    inline static MissionRandomizer_Commands       sm_Cmds;

    static bool
    Process (scrProgram *program, scrThreadContext *ctx)
    {
        return ProcessComponents<sm_Globals, sm_Order, sm_Flow,
                                 sm_PlayerSwitcher, sm_Cmds> (program, ctx);
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
