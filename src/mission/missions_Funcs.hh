#pragma once

#include "mission/missions_Globals.hh"
#include "missions_YscUtils.hh"
#include "rage.hh"
#include <cstdint>

class YscFunctions
{
    inline static auto MissionTriggererProgram = YscUtils::AnyValidProgram<
        "mission_triggerer_a"_joaat, "mission_triggerer_b"_joaat,
        "mission_triggerer_c"_joaat, "mission_triggerer_d"_joaat>;

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

    /* RunCodeId (id, delay) */
    inline static YscUtils::ScriptFunction<uint32_t, uint32_t>
        RunCodeId{"2d 02 04 00 ? 38 00 25 eb 08 2a 06 ",
                           "flow_controller"_joaat};

    /* SetSafehouseUnlocked (id, unlocked) */
    inline static YscUtils::ScriptFunction<uint32_t, bool> SetSafehouseUnlocked{
        "2d 02 04 00 ? 38 00 25 0a 57 ? ? 2e 02 00 38 01 56 ? ? 38 00 5e ? ? ? "
        "46 ? ? 34 01 6e 2c 08 ? ? 55",
        "flow_controller"_joaat};

    /* ResetTrigEntityProofs () */
    inline static YscUtils::ScriptFunction<> ResetTrigEntityProofs{
        "2d 00 03 00 ? 6e 39 02 38 02 25 08 5b ? ? 38 02 5e ? ? ? 35",
        MissionTriggererProgram};

    /* CleanupTrigEntities () */
    inline static YscUtils::ScriptFunction<> CleanupTrigEntities{
        "2d 00 04 00 ? 6e 39 02 38 02 25 08 5b", MissionTriggererProgram};

    /* SetShopState - 2d 03 05 00 ?? 38 ?? 56 ?? ?? 38 00 6e 6e */
    inline static YscUtils::ScriptFunction<uint32_t, bool, uint32_t>
        SetShopState{"2d 03 05 00 ? 38 ? 56 ? ? 38 00 6e 6e",
                     "flow_controller"_joaat};

    /* SetPlayerFreezeState (state) */
    inline static YscUtils::ScriptFunction<bool> SetPlayerFreezeState{
        "2d 01 05 00 ? 2c ? ? ? 2c ? ? ? 06 56 ? ? 38 ? 6f", "main"_joaat};
};
