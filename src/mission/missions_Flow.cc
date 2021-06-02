#include "missions_Flow.hh"

#include "CMath.hh"
#include "Utils.hh"
#include "common/logger.hh"
#include "mission/missions_Funcs.hh"
#include "mission/missions_Globals.hh"
#include "mission/missions_PlayerSwitch.hh"
#include "mission/missions_YscUtils.hh"
#include "mission/missions_Cmds.hh"
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

            // Don't want a transition to happen right after loading the
            // game (if you load a save game for armenian1 or start a
            // new game for example). Just set the position.
            if ("GET_IS_LOADING_SCREEN_ACTIVE"_n() || bMissionRepeating)
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

    Rainbomizer::Logger::LogMessage (
        "Finished initialising Stat Watcher for mission: %d", rId);
}

/*******************************************************/
void
MissionRandomizer_Flow::FixMissionRepeatStructForRandomizedMission ()
{
    ePlayerIndex player = ePlayerIndex (MR::sm_PlayerSwitcher.GetDestPlayer ());

    Rainbomizer::Logger::LogMessage (
        "Updating Mission Repeat Info players - %x to %x, %x to %x",
        MR::sm_Globals.g_MissionRepeatInfo->Player, player,
        MR::sm_Globals.g_MissionRepeatInfo2->Player, player);

    MR::sm_Globals.g_MissionRepeatInfo->Player  = player;
    MR::sm_Globals.g_MissionRepeatInfo2->Player = player;
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

    LogPlayerPos (true);
    MR::sm_PlayerSwitcher.BeginSwitch (GenerateSwitcherContext (true));

    nLastPassedMissionTime    = MR::sm_Globals.g_LastPassedMissionTime;
    bMissionRepeating         = false;
    nPlayerIndexOnMissionFail = ePlayerIndex::PLAYER_UNKNOWN;
    bInitialCutsceneRemoved   = false;
    bMissionStartupFinished   = false;
    nMissionPtrsSema          = 2;

    FixMissionRepeatStructForRandomizedMission ();
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
MissionRandomizer_Flow::HandleHeistCrewRandomization (scrThreadContext *ctx)
{
    if (ctx->m_nScriptHash != "code_controller"_joaat || ctx->m_nIp != 0)
        return true;

    if (!"HAS_SCRIPT_WITH_NAME_HASH_LOADED"_n("jewelry_heist"_joaat))
        {
            "REQUEST_SCRIPT_WITH_NAME_HASH"_n("jewelry_heist"_joaat);
            return false;
        }

    Rainbomizer::Logger::LogMessage ("Randomizing heist crews");

    *MR::sm_Globals.Crew_Dead_Bitset     = 0; // revive all dead crew members.
    *MR::sm_Globals.Crew_Unlocked_Bitset = 0xFFFFFFFF; // unlock all

    for (int heist = int (eHeistId::HEIST_JEWELRY);
         heist <= int (eHeistId::HEIST_FINALE); heist++)
        {
            SetFlowControlVariableForHeist (eHeistId (heist), true);
            YscFunctions::VerifyAndReplaceInvalidCrewMembers (eHeistId (heist));

            if (heist == int (eHeistId::HEIST_RURAL))
                continue;

            SetFlowControlVariableForHeist (eHeistId (heist), false);
            YscFunctions::VerifyAndReplaceInvalidCrewMembers (eHeistId (heist));
        }

    "SET_SCRIPT_WITH_NAME_HASH_AS_NO_LONGER_NEEDED"_n("jewelry_heist"_joaat);
    return true;
}

/*******************************************************/
bool
MissionRandomizer_Flow::HandleCutscenesForRandomizedMission ()
{
    if (!bInitialCutsceneRemoved)
        {
            if ("IS_CUTSCENE_ACTIVE"_n())
                {
                    "REMOVE_CUTSCENE"_n();
                    return false;
                }
            bInitialCutsceneRemoved = true;
        }

    auto cut = RandomizedMission->Data.sCutscene;
    if (bMissionRepeating || cut == std::string ("NONE")
        || "HAS_THIS_CUTSCENE_LOADED"_n(cut))
        return true;

    "REQUEST_CUTSCENE"_n(cut, 8);

    return false;
}

/*******************************************************/
bool
MissionRandomizer_Flow::OnMissionStart ()
{
    if (bMissionStartupFinished)
        return true;

    if (!HandleCutscenesForRandomizedMission ())
        return false;

    // Set player if mission was replayed
    if (bMissionRepeating
        && MR::sm_Globals.GetCurrentPlayer () == nPlayerIndexBeforeMission)
        MR::sm_PlayerSwitcher.BeginSwitch (GenerateSwitcherContext (true));

    if ("GET_IS_LOADING_SCREEN_ACTIVE"_n())
        {
            "SHUTDOWN_LOADING_SCREEN"_n();
            "DO_SCREEN_FADE_IN"_n(3000);
        }

    // Need to do this again for mission fails.
    FixMissionRepeatStructForRandomizedMission ();
    InitStatWatcherForRandomizedMission ();
    MR::sm_Cmds.OnMissionStart (OriginalMission->nHash,
                                RandomizedMission->nHash);

    bMissionStartupFinished = true;
    return false;
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

    MR::sm_Cmds.OnMissionEnd (pass, OriginalMission->nHash,
                              RandomizedMission->nHash);

    bMissionRepeating         = true;
    nPlayerIndexOnMissionFail = MR::sm_Globals.GetCurrentPlayer ();
    bMissionStartupFinished   = false;
    bTriggererCleanupFinished = false;
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
    if (!HandleHeistCrewRandomization (ctx))
        return false;

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

    MR::sm_Globals.GetMfInt (controlId) = controlValue;
}

/*******************************************************/
void
MissionRandomizer_Flow::SetHeistFlowControlVariablesForMission ()
{
    switch (RandomizedMission->nHash)
        {
        case "agency_heist3a"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_AGENCY, true);
            break;

        case "agency_heist3b"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_AGENCY, false);
            break;

        case "docks_heista"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_DOCKS, true);
            break;

        case "docks_heistb"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_DOCKS, false);
            break;

        case "finale_heist2a"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_FINALE, false);
            break;

        case "finale_heist2b"_joaat:
            SetFlowControlVariableForHeist (eHeistId::HEIST_FINALE, true);
            break;
        }

    // Set the board init state to 0xFF to set all the bits to true for
    // jewelry_heist and rural_bank_heist.
    MR::sm_Globals.g_BoardInitStateBitset.Init ();
    MR::sm_Globals.g_BoardInitStateBitset.Set (0xFF);
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
        || (ctx->m_nScriptHash != RandomizedMission->nHash
            && ctx->m_nScriptHash != "selector"_joaat))
        return;

    MR::sm_Globals.g_CurrentMission.Set (RandomizedMission->nId);
    MR::sm_Order.RestoreOriginalMissionInfo (MR::sm_Globals.g_CurrentMission);
    SetHeistFlowControlVariablesForMission ();
}

/*******************************************************/
void
MissionRandomizer_Flow::ClearVariables (scrThreadContext *ctx)
{
    if (!RandomizedMission || !OriginalMission
        || (ctx->m_nScriptHash != RandomizedMission->nHash
            && ctx->m_nScriptHash != "selector"_joaat))
        return;

    MR::sm_Order.ReapplyRandomMissionInfo (MR::sm_Globals.g_CurrentMission);
    MR::sm_Globals.g_CurrentMission.Set (nPreviousCurrentMission);
    SetHeistFlowControlVariables ();
}
