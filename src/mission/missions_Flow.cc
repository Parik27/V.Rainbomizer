#include "missions_Flow.hh"

#include "CMath.hh"
#include "Utils.hh"
#include "common/logger.hh"
#include "mission/missions_Funcs.hh"
#include "mission/missions_Globals.hh"
#include "mission/missions_YscUtils.hh"
#include "missions.hh"
#include "scrThread.hh"
#include <cstdint>
#include <stdio.h>
#include <utility>

using MR = MissionRandomizer_Components;

/*******************************************************/
auto
MissionRandomizer_Flow::GenerateSwitcherContext (bool start)
{
    MissionRandomizer_PlayerSwitcher::Context ctx;
    ctx.transitionType = ctx.PLAYER_SWITCHER;

    // For mission start
    if (start)
        {
            ctx.destPos = RandomizedMission->Data.vecStartCoords;
            ctx.destPlayer
                = RandomizedMission->Data.vecStartCoords.GetRandomPlayer ();

            switch (RandomizedMission->nHash)
                {
                    // These missions don't have a proper start, so allow fades
                    // only.
                case "prologue1"_joaat:
                case "armenian1"_joaat:
                    ctx.transitionType = ctx.FADE_OUT_ONLY;
                    ctx.noSetPos       = true;
                    break;
                }

            switch (OriginalMission->nHash)
                {
                    // Don't want a transition to happen right after loading the
                    // game (if you load a save game for armenian1 or start a
                    // new game for example). Just set the position.
                case "prologue1"_joaat:
                case "armenian1"_joaat: ctx.transitionType = ctx.NO_TRANSITION;
                }

            if (bMissionRepeating)
                ctx.transitionType = ctx.NO_TRANSITION;
        }
    else
        {
            ctx.destPos = OriginalMission->Data.vecEndCoords;
            ctx.destPlayer
                = OriginalMission->Data.vecEndCoords.GetRandomPlayer ();

            switch (OriginalMission->nHash)
                {
                case "prologue1"_joaat:
                case "finalea"_joaat:
                case "finaleb"_joaat:
                case "finalec2"_joaat: ctx.transitionType = ctx.FADE_OUT_ONLY;
                }

            switch (RandomizedMission->nHash)
                {
                case "prologue1"_joaat: ctx.transitionType = ctx.FADES;
                }
        }

    return ctx;
}

/*******************************************************/
void
MissionRandomizer_Flow::Reset ()
{    
    MR::sm_Order.Reset ();
    MR::sm_PlayerSwitcher.Reset ();

    OriginalMission   = nullptr;
    RandomizedMission = nullptr;
}

/*******************************************************/
void
MissionRandomizer_Flow::InitStatWatcherForRandomizedMission ()
{
    auto missionInfo = RandomizedMission;

    if (missionInfo->nHash == "finalec2"_joaat)
        {
            missionInfo = MR::sm_Order.GetMissionInfo ("finalec1"_joaat);
            if (!missionInfo)
                missionInfo = RandomizedMission;
        }

    uint32_t rId       = missionInfo->nId;
    auto     rDef_curr = missionInfo->pDef;
    auto     rDef_orig = &missionInfo->DefCopy;

    uint32_t prevFlags
        = std::exchange (rDef_curr->nMissionFlags, rDef_orig->nMissionFlags);

    Rainbomizer::Logger::LogMessage (
        "Initialising Stat Watcher for mission: %d", rId);

    YscFunctions::InitStatWatcherForMission (rId);
    rDef_curr->nMissionFlags = prevFlags;
}

/*******************************************************/
bool
MissionRandomizer_Flow::PreMissionStart ()
{
    Rainbomizer::Logger::LogMessage ("Mission %s (%u) to %s (%u) ",
                                     OriginalMission->Data.sName,
                                     OriginalMission->nHash,
                                     RandomizedMission->Data.sName,
                                     RandomizedMission->nHash);

    nPlayerIndexBeforeMission = MR::sm_Globals.GetCurrentPlayer ();

    LogPlayerPos(true);
    MR::sm_PlayerSwitcher.BeginSwitch (GenerateSwitcherContext(true));

    nLastPassedMissionTime = MR::sm_Globals.g_LastPassedMissionTime;
    bMissionRepeating      = false;
    nMissionPtrsSema       = 2;

    InitStatWatcherForRandomizedMission ();
    SetHeistFlowControlVariables ();

    return true;
}

/*******************************************************/
void
MissionRandomizer_Flow::HandleCurrentMissionChanges ()
{
    if (MR::sm_Globals.g_CurrentMission != nPreviousCurrentMission)
        {
            Rainbomizer::Logger::LogMessage (
                "g_CurrentMission changed: %u to %u", nPreviousCurrentMission,
                uint32_t (MR::sm_Globals.g_CurrentMission));

            nPreviousCurrentMission = MR::sm_Globals.g_CurrentMission;

            if (MR::sm_Globals.g_CurrentMission != -1u)
                {
                    OriginalMission = MR::sm_Order.GetMissionInfoFromId (
                        nPreviousCurrentMission);

                    if (OriginalMission)
                        RandomizedMission = MR::sm_Order.GetMissionInfo (
                            MR::sm_Order.GetRandomMissionHash (
                                OriginalMission->nHash));

                    if (OriginalMission && RandomizedMission
                        && PreMissionStart ())
                        return; // Success
                }
            bMissionRepeating = false;
        }
}

/*******************************************************/
bool
MissionRandomizer_Flow::OnMissionStart ()
{
    *MR::sm_Globals.FLAG_PLAYER_PED_INTRODUCED_M = true;
    *MR::sm_Globals.FLAG_PLAYER_PED_INTRODUCED_T = true;
    *MR::sm_Globals.FLAG_PLAYER_PED_INTRODUCED_F = true;
    *MR::sm_Globals.SP0_AVAILABLE                = true;
    *MR::sm_Globals.SP1_AVAILABLE                = true;
    *MR::sm_Globals.SP2_AVAILABLE                = true;

    YscFunctions::SetBuildingState (70, 1, 1, 1, 0);
    YscFunctions::SetBuildingState (71, 1, 1, 1, 0);
    YscFunctions::SetBuildingState (73, 1, 1, 1, 0);
    YscFunctions::SetBuildingState (72, 0, 1, 1, 0);
    YscFunctions::SetBuildingState (74, 0, 1, 1, 0);
    YscFunctions::SetDoorState (62, 1);
    YscFunctions::SetDoorState (63, 1);

    // Most missions need this to not get softlocked.
    if ("IS_CUTSCENE_ACTIVE"_n())
        {
            "REMOVE_CUTSCENE"_n();
            return false;
        }

    // Set player if mission was replayed
    if (bMissionRepeating
        && MR::sm_Globals.GetCurrentPlayer () == nPlayerIndexBeforeMission)
        {
            MR::sm_PlayerSwitcher.BeginSwitch (GenerateSwitcherContext (true));
        }

    if ((OriginalMission->nHash == "prologue1"_joaat
         || OriginalMission->nHash == "armenian1"_joaat)
        && !bMissionRepeating)
        {
            "SHUTDOWN_LOADING_SCREEN"_n();
            "DO_SCREEN_FADE_IN"_n(0);
        }

    // Need to do this again for mission fails.
    InitStatWatcherForRandomizedMission ();
    MR::sm_Globals.g_ForceWalking.Set (0);

    return true;
}

/*******************************************************/
void
MissionRandomizer_Flow::LogPlayerPos (bool start)
{
    FILE *f    = (start) ? mStartCoordsFile : mEndCoordsFile;
    auto *info = (start) ? OriginalMission : RandomizedMission;

    Vector3_native pos = NativeManager::InvokeNative<Vector3_native> (
        "GET_ENTITY_COORDS"_joaat, "PLAYER_PED_ID"_n(), 0);
    float heading = "GET_ENTITY_HEADING"_n("PLAYER_PED_ID"_n());

    fprintf (f, "%s: %f %f %f %f\n", info->Data.sName, pos.x, pos.y, pos.z,
             heading);
    fflush (f);
}

/*******************************************************/
bool
MissionRandomizer_Flow::OnMissionEnd (bool pass)
{
    bCallMissionEndNextFrame = false;

    Rainbomizer::Logger::LogMessage (
        "Mission %s, g_LastPassedMission = %u, g_CurrentMission = %u,"
        "g_LastPassedMissionTime = %u (prev = %u)",
        pass ? "Passed" : "Failed",
        uint32_t (MR::sm_Globals.g_LastPassedMission), nPreviousCurrentMission,
        uint32_t (MR::sm_Globals.g_LastPassedMissionTime),
        nLastPassedMissionTime);

    if (pass)
        {
            LogPlayerPos (false);
            MR::sm_PlayerSwitcher.BeginSwitch (GenerateSwitcherContext (false));
        }
    else
        {
#if (0)
            if (nPlayerIndexBeforeMission != ePlayerIndex::PLAYER_UNKNOWN)
                MR::sm_PlayerSwitcher.SetCurrentPlayer (
                    nPlayerIndexBeforeMission);
#endif
        }

    bMissionRepeating = true;
    return true;
}

/*******************************************************/
bool
MissionRandomizer_Flow::WasMissionPassed ()
{
    return MR::sm_Globals.g_LastPassedMission == nPreviousCurrentMission
           && MR::sm_Globals.g_LastPassedMissionTime != nLastPassedMissionTime;
}

/*******************************************************/
bool
MissionRandomizer_Flow::Process (scrProgram *program, scrThreadContext *ctx)
{
    if (!MR::sm_Order.IsInitialised ())
        return true;

    HandleCurrentMissionChanges ();

    if (bCallMissionEndNextFrame && ctx->m_nState != eScriptState::KILLED)
        return OnMissionEnd (WasMissionPassed ());

    if (!RandomizedMission || !OriginalMission
        || ctx->m_nScriptHash != RandomizedMission->nHash)
        return true;

    // Defer Mission pass callback to the next valid script execution
    if (ctx->m_nState == eScriptState::KILLED)
        {
            bCallMissionEndNextFrame = true;
            return true;
        }

    // Script just started
    if (ctx->m_nIp == 0)
        return OnMissionStart ();

    return true;
}

/*******************************************************/
void
MissionRandomizer_Flow::SetFlowControlVariableForHeist (eHeistId id,
                                                        bool     approach)
{
    // Similar to a function in the heist controllers.
    eHeistApproach           controlValue;
    eMissionFlowControlIntId controlId;

    switch (id)
        {
        case eHeistId::HEIST_JEWELRY:
            controlId    = INT_HEIST_CHOICE_JEWEL;
            controlValue = approach ? JEWEL_STEALTH : JEWEL_HIGH_IMPACT;
            break;

        case eHeistId::HEIST_AGENCY:
            controlId    = INT_HEIST_CHOICE_AGENCY;
            controlValue = approach ? AGENCY_FIRETRUCK : AGENCY_HELICOPTER;
            break;

        case eHeistId::HEIST_RURAL:
            controlId    = INT_HEIST_CHOICE_RURAL;
            controlValue = RURAL_NO_TANK;
            break;

        case eHeistId::HEIST_FINALE:
            controlId    = INT_HEIST_CHOICE_FINALE;
            controlValue = approach ? FINALE_HELI : FINALE_TRAFFCONT;
            break;

        case eHeistId::HEIST_DOCKS:
            controlId    = INT_HEIST_CHOICE_DOCKS;
            controlValue = approach ? DOCKS_BLOW_UP_BOAT : DOCKS_DEEP_SEA;
            break;
        }

    YscFunctions::SetMfControlInt (controlId, controlValue);
}

/*******************************************************/
void
MissionRandomizer_Flow::SetHeistFlowControlVariablesForMission ()
{
    switch (RandomizedMission->nHash)
        {
        case "agency_heist3a"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_AGENCY, false);
            break;

        case "agency_heist3b"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_AGENCY, true);
            break;

        case "docks_heista"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_DOCKS, true);
            break;

        case "docks_heistb"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_DOCKS, false);
            break;

        case "finale_heist2a"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_FINALE, true);
            break;

        case "finale_heist2b"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_FINALE, false);
            break;
        }
}

/*******************************************************/
void
MissionRandomizer_Flow::SetHeistFlowControlVariables ()
{
#define SET_FLOW_CONTROL_VAR(id, choice)                                       \
    SetFlowControlVariableForHeist (eHeistId::id, choices.choice)

    auto choices = MR::sm_Order.GetChoices ();

    SET_FLOW_CONTROL_VAR (HEIST_JEWELRY, JewelStealth);
    SET_FLOW_CONTROL_VAR (HEIST_AGENCY, AgencyFiretruck);
    SET_FLOW_CONTROL_VAR (HEIST_DOCKS, DocksBlowUpBoat);
    SET_FLOW_CONTROL_VAR (HEIST_FINALE, FinaleHeli);
    SET_FLOW_CONTROL_VAR (HEIST_RURAL, FinaleHeli);

#undef SET_FLOW_CONTROL_VAR
}

/*******************************************************/
void
MissionRandomizer_Flow::SetVariables (scrThreadContext *ctx)
{
    if (!RandomizedMission || !OriginalMission
        || ctx->m_nScriptHash != RandomizedMission->nHash)
        return;

    MR::sm_Globals.g_CurrentMission.Set (RandomizedMission->nId);
    SetHeistFlowControlVariablesForMission ();
}

/*******************************************************/
void
MissionRandomizer_Flow::ClearVariables (scrThreadContext *ctx)
{
    if (!RandomizedMission || !OriginalMission
        || ctx->m_nScriptHash != RandomizedMission->nHash)
        return;

    MR::sm_Globals.g_CurrentMission.Set (nPreviousCurrentMission);
    SetHeistFlowControlVariables ();
}
