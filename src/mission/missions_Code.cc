#include "missions_Code.hh"
#include "common/logger.hh"
#include "missions.hh"
#include "scrThread.hh"
#include <cstdint>
#include "common/ysc.hh"

using MR = MissionRandomizer_Components;

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyMissionPassFix (YscUtilsOps &utils)
{
    if (!MR::sm_Data.IsValidMission (utils.GetProgram ()->m_nScriptHash))
        return false;

    utils.Init ("56 ? ? 55 ? ? 6f 54");
    utils.NOP (/*Offset=*/6, /*Size=*/4);

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyStatWatcherFix (YscUtilsOps &utils)
{
    if (!utils.IsAnyOf ("flow_controller"_joaat))
        return false;

    /* This code pattern will nop the function call (of size 4, at offset
     * 12) to terminate the mission_stat_watcher for certain missions. This
     * will allow the Mission Passed screen to be displayed after missions
     * that don't normally have one like fbi4_intro */

    utils.Init ("6f 54 ? ? 6f 54 ? ? 5d ? ? ? 5d ? ? ? 2e 01 00");
    utils.NOP (/*Offset=*/12, /*Size=*/4);

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyCreditsFix_FinaleAB (YscUtilsOps &utils)
{
    if (!utils.IsAnyOf ("finalea"_joaat, "finaleb"_joaat))
        return false;

    utils.Init ("50 ? ? 6e 57 ? ? 2c ? ? ? 61 ? ? ? 59 ? ?");
    utils.NOP (/*Offset=*/0, /*Size=*/4);
    utils.Write (/*Offset=*/4, utils.OpCode(J));

    utils.Init ("59 ? ? 2c ? ? ? 06 56 ? ? 43 88 13 2c 04 ? ? 2c ? ? ? 56");
    utils.NOP (/*Offset=*/0, /*Size=*/25);

    utils.Init ("2d 00 02 00 ? 2c ? ? ? 06 56 ? ? 6e 2c 04");
    utils.Write (/*Offset=*/10, utils.OpCode (J));

    utils.Init ("5f ? ? ? 6e 57 ? ? 2c ? ? ? 61 ? ? ? 59 ? ?");
    utils.Write (/*Offset=*/5, utils.OpCode (J));

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyCreditsFix_FinaleC2 (YscUtilsOps &utils)
{
    static uint8_t j_0x24[] = {utils.OpCode (J), 0x16, 0x00};

    if (!utils.IsAnyOf ("finalec2"_joaat))
        return false;

    utils.Init ("2d 00 02 00 ? 6f 2c 04 ? ? 6e");
    utils.WriteBytes (/*Offset=*/5, j_0x24);

    utils.Init ("5f ? ? ? 06 56 ? ? 2c ? ? ? 61");
    utils.Write (/*Offset=*/5, utils.OpCode(J));

    utils.Init ("5a ? ? 43 88 13 2c 04 ? ? 2c ? ? ? 06 56 ? ? 6e 2c 04 ? ? 55");
    utils.NOP (/*Offset=*/0, /*Size=*/26);

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyTriggererWaitFix (YscUtilsOps &utils)
{
    // A few missions wait for the triggerer to create the peds that are
    // required for the mission. They don't do this for mission replay, so this
    // makes it think that the mission is being replayed. I don't know if there
    // are any side effects of this other than it just making things work, but
    // hopefully there aren't any. It could be made specific to only the
    // missions that wait for the triggerer, but you would need to go through
    // all the missions and find out which ones are broken.

    static uint8_t return_1[] = {
        utils.OpCode (PUSH_CONST_1),   // PUSH_CONST_1
        utils.OpCode (LEAVE), 0x1, 0x1 // LEAVE 0x1, 0x1
    };

    if (!MR::sm_Data.IsValidMission (utils.GetProgram ()->m_nScriptHash))
        return false;

    if (!utils.IsAnyOf ("agency_heist1"_joaat, "agency_heist3a"_joaat,
                        "jewelry_prep1a"_joaat, "jewelry_prep1b"_joaat,
                        "jewelry_prep2a"_joaat, "family2"_joaat,
                        "fbi4_intro"_joaat, "fbi5a"_joaat, "fbi4"_joaat,
                        "lester1"_joaat, "agency_prep1"_joaat, "family6"_joaat,
                        "docks_prep1"_joaat, "rural_bank_prep1"_joaat,
                        "docks_prep2b"_joaat))
        return false;

    utils.Init ("2d 01 03 00 ? 38 ? 06 2a 56");
    utils.WriteBytes (/*Offset=*/5, return_1);

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyPrepNoRepeatFix (YscUtilsOps &utils)
{
    // A few missions don't restart when you click replay, instead they put you
    // somewhere near the original location and expect you to restart it
    // yourself. This is undesired behaviour in mission randomizer since you
    // would want to be able to restart these missions.

    if (!utils.IsAnyOf ("replay_controller"_joaat))
        return false;

    uint8_t shellCode[]
        = {utils.OpCode(PUSH_CONST_1), utils.OpCode(LEAVE), 0x0, 0x1};

    utils.Init ("2d 00 03 00 ? 6e 5d ? ? ? 06 56 ? ? 5d ? ? ? 6f");
    utils.WriteBytes (/*Offset=*/5, shellCode);

    //utils.Write (/*Offset=*/11, utils.OpCode(J));

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyQuickSkipsPatch (YscUtilsOps &utils)
{
    /* This patch removes the 2 mission fails with no progress made checks from
     * flow_controller's function that sets the can skip global variable for
     * replay_controller */

    if (!utils.IsAnyOf ("flow_controller"_joaat)
        || !MR::Config ().EnableFastSkips)
        return false;

    utils.Init ("5e ? ? ? 46 ? ? 46 ? ? 34 ? 41 01 70 5a ? ?");
    utils.NOP (/*Offset=*/15, /*Size=*/3);

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplySolomonCamFix (YscUtilsOps &utils)
{
    /* Mr. Richards. The trigger for this mission starts with a camera pan up
     * before the mission starts. However, it seems again that the original
     * mission script is responsible for resetting it, so it gets stuck there
     * for anything else */

    if (!utils.IsAnyOf ("mission_triggerer_c"_joaat))
        return false;

    utils.Init ("5d ? ? ? 2c ? ? ? 51 ? ? 50");
    auto nopEnd = utils.GetWorkingIp () + 11;

    utils.Init ("29 00 00 48 42 6e 70 2c");
    utils.NOP (/*Offset=*/7, /*Size=*/nopEnd - utils.GetWorkingIp() + 7);

    Rainbomizer::Logger::LogMessage ("solomon1 nop size: %d",
                                     nopEnd - utils.GetWorkingIp() + 7);

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyBuildingStatePatch (YscUtilsOps &utils)
{
    /* This patch makes it so all building/door changes are applied immediately,
     * by removing the checks for bits/switch states/load scenes etc.
     */

    if (!utils.IsAnyOf ("flow_controller"_joaat))
        return false;

    utils.Init ("56 ? ? 6f 39 0a 38 0a 56 ? ?");
    utils.NOP (/*Offset=*/8, /*Size=*/3);

    utils.Init ("38 69 56 ? ? 37 09 41 03 62 ?");
    utils.NOP (/*Offset=*/2, /*Size=*/3);

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyReplayControllerFix (YscUtilsOps &utils)
{
    /* This patch fixes softlock on failing finale_intro (randomized or
     * original) */

    if (!utils.IsAnyOf ("replay_controller"_joaat))
        return false;

    uint8_t shellCode[]
        = {utils.OpCode(GLOBAL_U24_LOAD), 0x00, 0x00, 0x00};

    utils.Init (
        "2d 00 ? ? ? 5d ? ? ? 56 ? ? 5e ? ? ? 40 ? 6e 5d ? ? ? 39 ? 38");
    utils.NOP (/*Offset=*/12, /*Size=*/11);

    uint32_t globalIdx = MR::sm_Globals.g_CurrentMission.GetIndex ();
    memcpy (&shellCode[1], &globalIdx, 3);

    utils.WriteBytes (/*Offset=*/12, shellCode);

    return true;
}

/*******************************************************/
bool
MissionRandomizer_CodeFixes::ApplyProloguePlayerResetFix (YscUtilsOps &utils)
{
    /* This patch fixes randomized Prologue resetting player */
    // 6f 2c 04 ? ? 2c ? ? ? 2c ? ? ? 56 ? ? 50 ? ? 6f 2c 08 ? ? 5d ? ? ?

    if (!utils.IsAnyOf ("prologue1"_joaat))
        return false;

    utils.Init ("6f 2c 04 ? ? 2c ? ? ? 2c ? ? ? 56 ? ? 50 ? ? 6f 2c 08 ? ? 5d");
    utils.NOP (/*Offset=*/16, /*Size=*/12);

    return true;
}

/*******************************************************/
void
MissionRandomizer_CodeFixes::Initialise ()
{
    YscCodeEdits::Add ("Prologue Player Reset Fix",
                       ApplyProloguePlayerResetFix);
    YscCodeEdits::Add ("Stat Watcher Fix", ApplyStatWatcherFix);
    YscCodeEdits::Add ("Finale A/B Credits Fix", ApplyCreditsFix_FinaleAB);
    YscCodeEdits::Add ("Finale C Credits Fix", ApplyCreditsFix_FinaleC2);
    YscCodeEdits::Add ("Mission Pass Fix", ApplyMissionPassFix);
    YscCodeEdits::Add ("Triggerer Wait Fix", ApplyTriggererWaitFix);
    YscCodeEdits::Add ("Prep No Repeat Fix", ApplyPrepNoRepeatFix);
    YscCodeEdits::Add ("Quick Skips Patch", ApplyQuickSkipsPatch);
    YscCodeEdits::Add ("Solomon Cam Fix", ApplySolomonCamFix);
    YscCodeEdits::Add ("Building State Patch", ApplyBuildingStatePatch);
    YscCodeEdits::Add ("Replay Controller Softlock Fix",
                       ApplyReplayControllerFix);
}
