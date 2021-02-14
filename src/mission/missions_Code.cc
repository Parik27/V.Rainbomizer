#include "missions_Code.hh"
#include "missions.hh"
#include "scrThread.hh"
#include <cstdint>

using MR = MissionRandomizer_Components;

/*******************************************************/
void
MissionRandomizer_CodeFixes::PrintStatus (YscUtilsOps &    utils,
                                          std::string_view fixName)
{
    Rainbomizer::Logger::LogMessage ("%s: %s", fixName.data (),
                                     utils ? "Failed" : "Succeeded");
}

/*******************************************************/
void
MissionRandomizer_CodeFixes::ApplyMissionPassFix (YscUtilsOps &utils)
{
    if (!MR::sm_Data.IsValidMission (utils.GetProgram ()->m_nScriptHash))
        return;

    utils.Init ("56 ? ? 55 ? ? 6f 54");
    utils.NOP (/*Offset=*/6, /*Size=*/4);

    PrintStatus (utils, "Mission Pass Fix");
}

/*******************************************************/
void
MissionRandomizer_CodeFixes::ApplyStatWatcherFix (YscUtilsOps &utils)
{
    if (!utils.IsAnyOf ("flow_controller"_joaat))
        return;

    /* This code pattern will nop the function call (of size 4, at offset
     * 12) to terminate the mission_stat_watcher for certain missions. This
     * will allow the Mission Passed screen to be displayed after missions
     * that don't normally have one like fbi4_intro */

    utils.Init ("6f 54 ? ? 6f 54 ? ? 5d ? ? ? 5d ? ? ? 2e 01 00");
    utils.NOP (/*Offset=*/12, /*Size=*/4);
    
    PrintStatus (utils, "mission_stat_watcher fix: ");
}

/*******************************************************/
void
MissionRandomizer_CodeFixes::ApplyCreditsFix_FinaleAB (YscUtilsOps &utils)
{
    if (!utils.IsAnyOf ("finalea"_joaat, "finaleb"_joaat))
        return;

    utils.Init ("50 ? ? 6e 57 ? ? 2c ? ? ? 61 ? ? ? 59 ? ?");
    utils.NOP (/*Offset=*/0, /*Size=*/4);
    utils.Write (/*Offset=*/4, YscOpCode::J);

    utils.Init ("59 ? ? 2c ? ? ? 06 56 ? ? 43 88 13 2c 04 ? ? 2c ? ? ? 56");
    utils.NOP (/*Offset=*/0, /*Size=*/25);

    utils.Init ("2d 00 02 00 ? 2c ? ? ? 06 56 ? ? 6e 2c 04");
    utils.Write (/*Offset=*/10, YscOpCode::J);

    utils.Init ("5f ? ? ? 6e 57 ? ? 2c ? ? ? 61 ? ? ? 59 ? ?");
    utils.Write (/*Offset=*/5, YscOpCode::J);

    PrintStatus (utils, "Credits fix for finale a/b");
}

/*******************************************************/
void
MissionRandomizer_CodeFixes::ApplyCreditsFix_FinaleC2 (YscUtilsOps &utils)
{
    static constexpr uint8_t j_0x24[] = {0x55, 0x16, 0x00};

    if (!utils.IsAnyOf ("finalec2"_joaat))
        return;

    utils.Init ("2d 00 02 00 ? 6f 2c 04 ? ? 6e");
    utils.WriteBytes (/*Offset=*/5, j_0x24);

    utils.Init ("5f ? ? ? 06 56 ? ? 2c ? ? ? 61");
    utils.Write (/*Offset=*/5, YscOpCode::J);

    utils.Init ("5a ? ? 43 88 13 2c 04 ? ? 2c ? ? ? 06 56 ? ? 6e 2c 04 ? ? 55");
    utils.NOP (/*Offset=*/0, /*Size=*/26);

    PrintStatus (utils, "Credits fix for finalec2");
}

/*******************************************************/
void
MissionRandomizer_CodeFixes::ApplyTriggererWaitFix (YscUtilsOps &utils)
{
    // A few missions wait for the triggerer to create the peds that are
    // required for the mission. They don't do this for mission replay, so this
    // makes it think that the mission is being replayed. I don't know if there
    // are any side effects of this other than it just making things work, but
    // hopefully there aren't any. It could be made specific to only the
    // missions that wait for the triggerer, but you would need to go through
    // all the missions and find out which ones are broken.
    
    static constexpr uint8_t return_1[] = {
        0x6f,          // PUSH_CONST_1
        0x2e, 0x1, 0x1 // LEAVE 0x1, 0x1
    };

    if (!MR::sm_Data.IsValidMission (utils.GetProgram ()->m_nScriptHash))
        return;

    if (!utils.IsAnyOf ("agency_heist1"_joaat, "agency_heist3a"_joaat,
                        "jewelry_prep1a"_joaat, "jewelry_prep1b"_joaat,
                        "jewelry_prep2a"_joaat, "family2"_joaat,
                        "fbi4_intro"_joaat, "fbi5a"_joaat, "fbi4"_joaat,
                        "lester1"_joaat, "agency_prep1"_joaat))
        return;
    
    utils.Init ("2d 01 03 00 ? 38 ? 06 2a 56");
    utils.WriteBytes (/*Offset=*/5, return_1);

    PrintStatus(utils, "Triggerer Wait Fix");
}

/*******************************************************/
void
MissionRandomizer_CodeFixes::ApplyPrepNoRepeatFix (YscUtilsOps &utils)
{
    // A few missions don't restart when you click replay, instead they put you
    // somewhere near the original location and expect you to restart it
    // yourself. This is undesired behaviour in mission randomizer since you
    // would want to be able to restart these missions.

    if (!utils.IsAnyOf ("replay_controller"_joaat))
        return;

    utils.Init ("2d 00 03 00 ? 6e 5d ? ? ? 06 56 ? ? 5d ? ? ? 6f");
    utils.Write (/*Offset=*/11, YscOpCode::J);

    PrintStatus (utils, "Prep No Repeat Fix");
}

/*******************************************************/
void
MissionRandomizer_CodeFixes::ApplyQuickSkipsPatch (YscUtilsOps &utils)
{
    /* This patch removes the 2 mission fails with no progress made checks from
     * flow_controller's function that sets the can skip global variable for
     * replay_controller */

    if (!utils.IsAnyOf ("flow_controller"_joaat)
        || !MR::Config ().EnableFastSkips)
        return;

    utils.Init ("5e ? ? ? 46 ? ? 46 ? ? 34 ? 41 01 70 5a ? ?");
    utils.NOP (/*Offset=*/15, /*Size=*/3);

    PrintStatus (utils, "Quick Skips Patch");
}
