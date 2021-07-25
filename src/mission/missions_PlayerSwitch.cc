#include "missions_PlayerSwitch.hh"

#include "CPed.hh"
#include "Patterns/Patterns.hh"
#include "common/logger.hh"
#include "mission/missions_Globals.hh"
#include "mission/missions_Funcs.hh"
#include "rage.hh"
#include "scrThread.hh"

#include <CTheScripts.hh>
#include <CStreaming.hh>

#include <utility>
#include <cstdint>
#include <ctime>

#include "missions.hh"

#include "missions_Globals.hh"
#include "peds/peds_Compatibility.hh"

using namespace NativeLiterals;

using MR = MissionRandomizer_Components;

bool
MissionRandomizer_PlayerSwitcher::Process (scrProgram *, scrThreadContext *)
{
    switch (m_nCurrentState)
        {
        case IDLE: return true;

        case TRANSITION_BEGIN:
            if (DoTransitionBegin ())
                m_nCurrentState = TRANSITION_ASCEND;
            break;

        case TRANSITION_ASCEND:
            if (DoTransitionProcess ())
                m_nCurrentState = PLAYER_SWITCH;
            break;

        case PLAYER_SWITCH:
            if (DoPlayerSwitch ())
                m_nCurrentState = TRANSITION_END;
            break;

        case TRANSITION_END:
            if (DoTransitionEnd ())
                m_nCurrentState = TRANSITION_DESCEND;
            break;

        case TRANSITION_DESCEND:
            if (DoTransitionProcessEnd ())
                m_nCurrentState = TRANSITION_CLEANUP;
            break;

        case TRANSITION_CLEANUP:
            if (DoTransitionCleanup ())
                m_nCurrentState = IDLE;
        }

    return false;
}
bool
MissionRandomizer_PlayerSwitcher::HasDescentFinished ()
{
    return !"IS_PLAYER_SWITCH_IN_PROGRESS"_n();
    return true;
}
uint32_t
MissionRandomizer_PlayerSwitcher::GetDestPlayer ()
{
    if (m_bDestPlayerSet)
        return *MissionRandomizer_GlobalsManager::PP_CURRENT_PED;
    return uint32_t (m_Context.destPlayer);
}
void
MissionRandomizer_PlayerSwitcher::BeginSwitch (Context ctx)
{
    if (m_nCurrentState != IDLE)
        return;

    m_bDestPlayerSet = false;

    Rainbomizer::Logger::LogMessage (
        "Beginning Player Switch: %f %f %f (%d) - %d", ctx.destPos.x,
        ctx.destPos.y, ctx.destPos.z, ctx.destPlayer, ctx.transitionType);

    m_nCurrentState = TRANSITION_BEGIN;
    m_Context       = ctx;
}
bool
MissionRandomizer_PlayerSwitcher::DoTransitionCleanup ()
{
    if (!m_Context.noSetPos)
        YscFunctions::SetPlayerFreezeState (false);
    return true;
}
bool
MissionRandomizer_PlayerSwitcher::DoTransitionProcessEnd ()
{
    switch (m_Context.transitionType)
        {
        case Context::PLAYER_SWITCHER: return HasDescentFinished ();

        case Context::FADES: return !"IS_SCREEN_FADING_IN"_n();

        case Context::FADE_OUT_ONLY:
        case Context::NO_TRANSITION: return true;
        }

    return true;
}
bool
MissionRandomizer_PlayerSwitcher::DoTransitionEnd ()
{
    switch (m_Context.transitionType)
        {
        case Context::PLAYER_SWITCHER:
            "_SWITCH_IN_PLAYER"_n("PLAYER_PED_ID"_n());
            "DO_SCREEN_FADE_IN"_n(1000);
            return true;

        case Context::FADES: "DO_SCREEN_FADE_IN"_n(1000); return true;

        case Context::FADE_OUT_ONLY:
        case Context::NO_TRANSITION: return true;
        }

    return true;
}
bool
MissionRandomizer_PlayerSwitcher::DoPlayerSwitch ()
{
    if (SetCurrentPlayer (m_Context.destPlayer))
        {
            if (!m_Context.noSetPos)
                {
                    "SET_PED_COORDS_KEEP_VEHICLE"_n("PLAYER_PED_ID"_n(),
                                                    m_Context.destPos.x,
                                                    m_Context.destPos.y,
                                                    m_Context.destPos.z);
                    YscFunctions::SetPlayerFreezeState (true);
                }
            m_bDestPlayerSet = true;
            return true;
        }

    return false;
}
bool
MissionRandomizer_PlayerSwitcher::DoTransitionProcess ()
{
    switch (m_Context.transitionType)
        {
        case Context::PLAYER_SWITCHER: return "_933BBEEB8C61B5F4"_n();

        case Context::FADE_OUT_ONLY:
        case Context::FADES: return !"IS_SCREEN_FADING_OUT"_n();

        case Context::NO_TRANSITION: return true;
        }

    return true;
}

bool
MissionRandomizer_PlayerSwitcher::IsPlayerSceneOnGoing ()
{
    return "IS_PLAYER_SWITCH_IN_PROGRESS"_n()
           || "_GET_NUMBER_OF_INSTANCES_OF_SCRIPT_WITH_NAME_HASH"_n(
               "player_timetable_scene"_joaat);
}

bool
MissionRandomizer_PlayerSwitcher::DoTransitionBegin ()
{
    if (IsPlayerSceneOnGoing ())
        {
            "ALLOW_PLAYER_SWITCH_PAN"_n();
            "ALLOW_PLAYER_SWITCH_OUTRO"_n();
            "ALLOW_PLAYER_SWITCH_DESCENT"_n();

            MR::AllowScriptOnPauseThisFrame ("player_controller_b"_joaat);
            MR::AllowScriptOnPauseThisFrame ("player_timetable_scene"_joaat);
            return false;
        }

    switch (m_Context.transitionType)
        {
        case Context::PLAYER_SWITCHER:
            "NEW_LOAD_SCENE_START_SPHERE"_n(m_Context.destPos.x,
                                            m_Context.destPos.y,
                                            m_Context.destPos.z, 15.0f, 2);

            "_SWITCH_OUT_PLAYER"_n("PLAYER_PED_ID"_n(), 0, 1);
            return true;

        case Context::FADE_OUT_ONLY:
        case Context::FADES: "DO_SCREEN_FADE_OUT"_n(1000); return true;

        case Context::NO_TRANSITION: return true;
        }

    return true;
}
bool
MissionRandomizer_PlayerSwitcher::SetCurrentPlayer (ePlayerIndex index)
{
    if (*MissionRandomizer_GlobalsManager::PP_CURRENT_PED == int (index))
        return true;

    if (!YscFunctions::SetCurrentPlayer.CanCall (false))
        return false;

    if (!YscFunctions::SetCurrentPlayer (index, 1))
        return true;

    bool success = CheckCurrentPlayer (index);

    if (!success)
        {
            static auto lastPrint = 0ll;
            if (time (NULL) - lastPrint > 2)
                {
                    lastPrint = time (NULL);
                    Rainbomizer::Logger::LogMessage (
                        "Failed to set player model. current player: %x, "
                        "dst player: %x",
                        *MissionRandomizer_GlobalsManager::PP_CURRENT_PED,
                        m_Context.destPlayer);
                }
        }

    return success;
}
bool
MissionRandomizer_PlayerSwitcher::CheckCurrentPlayer (ePlayerIndex index)
{
    if (*MissionRandomizer_GlobalsManager::PP_CURRENT_PED == int (index))
        return true;

    if (*MissionRandomizer_GlobalsManager::PP_CURRENT_PED
        == int (eCharacter::_))
        {
            uint32_t hash     = 0;
            auto *playerModel = PedRandomizerCompatibility::GetOriginalModel (
                CPedFactory::Get ()->pPlayer);

            switch (index)
                {
                case ePlayerIndex::PLAYER_MICHAEL:
                    hash = "player_zero"_joaat;
                    break;
                case ePlayerIndex::PLAYER_FRANKLIN:
                    hash = "player_one"_joaat;
                    break;
                case ePlayerIndex::PLAYER_TREVOR:
                    hash = "player_two"_joaat;
                    break;
                default: return false;
                }

            return playerModel && playerModel->m_nHash == hash;
        }

    return false;
}
