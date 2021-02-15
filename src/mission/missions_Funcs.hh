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

    /*SetBuildingState(buildingId, state, distanceCheck, refreshInterior,
     * force);*/
    inline static YscUtils::ScriptFunction<uint32_t, uint32_t, uint32_t,
                                           uint32_t, uint32_t>
        SetBuildingState{"2d 05 07 00 ? 38 00 25 c6 58 ? ? 5f ? ? ? 56 ? ? ",
                         "flow_controller"_joaat};

    /*InitStatWatcherForMission (missionId) */
    inline static YscUtils::ScriptFunction<uint32_t> InitStatWatcherForMission{
        "2d 01 07 00 ? 6e 39 03 38 ? 25 1a 57", "flow_controller"_joaat};

    /* SetMfControlInt (controlId, value) */
    inline static YscUtils::ScriptFunction<eMissionFlowControlIntId, uint32_t>
        SetMfControlInt{"2d 02 04 00 ? 38 00 25 0d 08 2a 06 56",
                        "flow_controller"_joaat};

    /* SetMfControlFlag (flagId, value) */
    inline static YscUtils::ScriptFunction<eFlowFlag, bool> SetMfControlFlag{
        "2d 02 04 00 ? 38 00 25 8d 08 2a 06 ", "flow_controller"_joaat};

    /* VerifyAndReplaceInvalidCrewMembers (heistId) */
    inline static YscUtils::ScriptFunction<eHeistId>
        VerifyAndReplaceInvalidCrewMembers{
            "2d 01 05 00 ? 38 ? 5d ? ? ? 5d ? ? ? 39 03 6e 39 04 ",
            "jewelry_heist"_joaat};

    /* UnlockWeapon (player, hash, unlock, p4) */
    inline static YscUtils::ScriptFunction<ePlayerIndex, uint32_t, bool, bool>
        UnlockWeapon{"2d 04 09 00 ? 38 00 5d ? ? ? 56 ? ? 38 00 38 01 5d",
                     "flow_controller"_joaat};

    /* AddContact (contactToAdd, phoneBookCharacter, add) */
    inline static YscUtils::ScriptFunction<eCharacter, eCharacter, bool>
        AddContact{"2d 03 ? 00 ? 38 00 54 ? ? 38 00 52",
                   "flow_controller"_joaat};

    /* ExecuteControlCode (id, delay) */
    inline static YscUtils::ScriptFunction<uint32_t, uint32_t>
        ExecuteControlCode{"2d 02 04 00 ? 38 00 25 eb 08 2a 06 ",
                           "flow_controller"_joaat};

    /* SetSafehouseUnlocked (id, unlocked) */
    inline static YscUtils::ScriptFunction<uint32_t, bool> SetSafehouseUnlocked{
        "2d 02 04 00 ? 38 00 25 0a 57 ? ? 2e 02 00 38 01 56 ? ? 38 00 5e ? ? ? "
        "46 ? ? 34 01 6e 2c 08 ? ? 55",
        "flow_controller"_joaat};
};
