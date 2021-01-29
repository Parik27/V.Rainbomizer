#pragma once

#include "mission/missions_Globals.hh"
#include "missions_YscUtils.hh"
#include "rage.hh"
#include <cstdint>

class YscFunctions
{
public:
    /*SetDoorState(ePlayerIndex, freeModel);*/
    inline static YscUtils::ScriptFunction<ePlayerIndex, uint32_t>
        SetCurrentPlayer{"2d 02 0b 00 ? 6e 52 ? ? 42 ? 2c ? ? ? "
                         "2c ? ? ? 56 ? ? 6f 2e 02 01",
                         "main"_joaat};

    /*SetDoorState(doorId, state);*/
    inline static YscUtils::ScriptFunction<uint32_t, uint32_t> SetDoorState{
        "2d 02 05 00 ? 38 00 25 ? 58 ? ? 5f", "flow_controller"_joaat};

    /*SetBuildingState(buildingId, state, p3, p4, p5);*/
    inline static YscUtils::ScriptFunction<uint32_t, uint32_t, uint32_t,
                                           uint32_t, uint32_t>
        SetBuildingState{"2d 05 07 00 ? 38 00 25 c6 58 ? ? 5f ? ? ? 56 ? ? ",
                         "flow_controller"_joaat};

    /*InitStatWatcherForMission (missionId) */
    inline static YscUtils::ScriptFunction<uint32_t> InitStatWatcherForMission{
        "2d 01 07 00 ? 6e 39 03 38 ? 25 1a 57", "flow_controller"_joaat};  
};
