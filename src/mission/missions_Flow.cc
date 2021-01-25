#include "missions_Flow.hh"

#include "common/logger.hh"
#include "missions.hh"
#include "scrThread.hh"

using MR = MissionRandomizer_Components;

void
MissionRandomizer_Flow::Reset ()
{
    MR::sm_Order.Reset ();
    MR::sm_PlayerSwitcher.Reset ();
}

void
MissionRandomizer_Flow::HandleCurrentMissionChanges ()
{
    if (MR::sm_Globals.GetCurrentMission () != nPreviousCurrentMission)
        {
            Rainbomizer::Logger::LogMessage (
                "g_CurrentMission changed: %u to %u", nPreviousCurrentMission,
                MR::sm_Globals.GetCurrentMission ());

            nPreviousCurrentMission = MR::sm_Globals.GetCurrentMission ();

            if (MR::sm_Globals.GetCurrentMission () != -1u)
                {
                    nOriginalMission = MR::sm_Order.GetOriginalMissionHash (
                        nPreviousCurrentMission);
                    nRandomizedMission
                        = MR::sm_Order.GetRandomMissionHash (nOriginalMission);

                    pOriginalMission
                        = &MR::sm_Data.GetMissionData (nOriginalMission);
                    pRandomizedMission
                        = &MR::sm_Data.GetMissionData (nRandomizedMission);

                    // Invalid missions
                    if (nOriginalMission == -1 || nRandomizedMission == -1
                        || !pOriginalMission || !pRandomizedMission)
                        {
                            Rainbomizer::Logger::LogMessage (
                                "Skipping mission randomization flow: %d",
                                nPreviousCurrentMission);

                            nOriginalMission   = -1;
                            nRandomizedMission = -1;
                            pOriginalMission   = nullptr;
                            pRandomizedMission = nullptr;

                            return;
                        }

                    Rainbomizer::Logger::LogMessage (
                        "Mission %s (%u) to %s (u)", pOriginalMission->sName,
                        nOriginalMission, pRandomizedMission->sName,
                        nRandomizedMission);

                    MR::sm_PlayerSwitcher.BeginSwitch (
                        pRandomizedMission->vecStartCoords,
                        pRandomizedMission->vecStartCoords.GetRandomPlayer (),
                        pOriginalMission->OrigFlags.NoTransition
                            || pRandomizedMission->RandFlags.NoTransition);
                }
            else
                {

                    nOriginalMission   = -1;
                    nRandomizedMission = -1;
                    pOriginalMission   = nullptr;
                    pRandomizedMission = nullptr;
                }
        }
}

bool
MissionRandomizer_Flow::OnMissionStart ()
{
    *MR::sm_Globals.FLAG_PLAYER_PED_INTRODUCED_M = true;
    *MR::sm_Globals.FLAG_PLAYER_PED_INTRODUCED_T = true;
    *MR::sm_Globals.FLAG_PLAYER_PED_INTRODUCED_F = true;
    *MR::sm_Globals.SP0_AVAILABLE                = true;
    *MR::sm_Globals.SP1_AVAILABLE                = true;
    *MR::sm_Globals.SP2_AVAILABLE                = true;

    // Most missions need this to not get softlocked.
    if ("IS_CUTSCENE_ACTIVE"_n())
        {
            "REMOVE_CUTSCENE"_n();
            return false;
        }

    if (pOriginalMission->OrigFlags.FadeIn
        || pRandomizedMission->RandFlags.FadeIn)
        {
            "SHUTDOWN_LOADING_SCREEN"_n();
            "DO_SCREEN_FADE_IN"_n(0);
        }

    return true;
}

bool
MissionRandomizer_Flow::OnMissionEnd (bool fail)
{
    return true;
}

bool
MissionRandomizer_Flow::Process (scrProgram *program, scrThreadContext *ctx)
{
    if (!MR::sm_Order.IsInitialised ())
        return true;

    HandleCurrentMissionChanges ();
    if (ctx->m_nIp == 0 && nRandomizedMission != -1u && nOriginalMission != -1u)
        return OnMissionStart ();

    if (ctx->m_nState == eScriptState::KILLED)
        return OnMissionEnd (/* TODO: fail parameter */ false);

    return true;
}
