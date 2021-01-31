#include "missions_Code.hh"
#include "missions.hh"

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

#if (0)
    utils.GetFirst ("6e 5d ? ? ? 56 ? ? 6f 54 ? ? 6f 6f ");
    utils.MakeNOP (5, 3);
    utils.Set (13, 0x6e);
    utils.Set (18, 0x6e);
    utils.Set (19, 0x6e);
    utils.MakeNOP (29, 3);

    utils.GetFirst ("56 ? ? 55 ? ? 6f 54 ? ? ");
    utils.MakeNOP (6, 3);

    utils.GetFirst ("2d 01 03 00 ? 38 ? 06 2a 56 ? ? ");
    utils.Set (5, 0x6f);
    utils.Set (6, 0x2e);
    utils.Set (7, 1);
    utils.Set (8, 1);
#endif
}

/*******************************************************/
void
MissionRandomizer_CodeFixes::ApplyCreditsFix_FinaleC2 (YscUtilsOps &utils)
{
    static constexpr uint8_t ret_0x0[] = {0x2e, 0x00, 0x00};
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
