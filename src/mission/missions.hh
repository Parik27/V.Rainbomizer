#pragma once

#include "missions_Order.hh"
#include "missions_Globals.hh"
#include "missions_PlayerSwitch.hh"
#include "missions_Flow.hh"
#include "missions_Data.hh"

#include "common/common.hh"

class MissionRandomizer_Components
{
public:
    inline static MissionRandomizer_Data sm_Data{
        Rainbomizer::Common::GetRainbomizerDataFile ("Missions.txt")};

    inline static MissionRandomizer_GlobalsManager sm_Globals;
    inline static MissionRandomizer_OrderManager   sm_Order;
    inline static MissionRandomizer_PlayerSwitcher sm_PlayerSwitcher;
    inline static MissionRandomizer_Flow           sm_Flow;

    static auto &
    Config ()
    {
        static struct Config
        {
            std::string Seed;
            bool        ForceSeedOnSaves;
            bool        LogMissionOrder;

            std::string ForcedMission;

        } m_Config;

        return m_Config;
    }
};
