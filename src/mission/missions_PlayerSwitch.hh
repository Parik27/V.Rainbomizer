#pragma once

#include "Patterns/Patterns.hh"
#include "common/logger.hh"
#include "mission/missions_Globals.hh"
#include "mission/missions_YscUtils.hh"
#include "mission/missions_Funcs.hh"
#include "rage.hh"
#include "scrThread.hh"

#include <CTheScripts.hh>
#include <CStreaming.hh>

#include <utility>
#include <cstdint>
#include <ctime>

#include "missions_Globals.hh"

using namespace NativeLiterals;

class MissionRandomizer_PlayerSwitcher
{
public:
    struct Context
    {
        ePlayerIndex destPlayer;
        
        bool        noSetPos = false;
        rage::Vec3V destPos;

        enum
        {
            PLAYER_SWITCHER,
            FADES,
            FADE_OUT_ONLY,
            NO_TRANSITION
        } transitionType
            = PLAYER_SWITCHER;
    };

private:
    
    enum
        {
            IDLE,
            TRANSITION_BEGIN,
            TRANSITION_ASCEND,
            PLAYER_SWITCH,
            TRANSITION_END,
            TRANSITION_DESCEND
        } m_nCurrentState
    = IDLE;

    bool m_bDestPlayerSet = true;
    
    Context m_Context;

    
    /*******************************************************/
    bool
    SetCurrentPlayer (ePlayerIndex index)
    {
        if (*MissionRandomizer_GlobalsManager::PP_CURRENT_PED == int (index))
            return true;

        if (!YscFunctions::SetCurrentPlayer.CanCall (false))
            return false;

        if (!YscFunctions::SetCurrentPlayer (index, 1))
            return true;

        bool success
            = *MissionRandomizer_GlobalsManager::PP_CURRENT_PED == int (index);

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

    /*******************************************************/
    bool
    DoTransitionBegin ()
    {
        switch (m_Context.transitionType)
            {
            case Context::PLAYER_SWITCHER:
                if ("IS_PLAYER_SWITCH_IN_PROGRESS"_n())
                    return false;

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

    /*******************************************************/
    bool
    DoTransitionProcess ()
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

    /*******************************************************/
    bool
    DoPlayerSwitch ()
    {
        if (SetCurrentPlayer (m_Context.destPlayer))
            {
                if (!m_Context.noSetPos)
                    "SET_PED_COORDS_KEEP_VEHICLE"_n("PLAYER_PED_ID"_n(),
                                                    m_Context.destPos.x,
                                                    m_Context.destPos.y,
                                                    m_Context.destPos.z);
                m_bDestPlayerSet = true;
                return true;
            }

        return false;
    }

    /*******************************************************/
    bool
    DoTransitionEnd ()
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

    /*******************************************************/
    bool
    DoTransitionProcessEnd ()
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

public:
    /*******************************************************/
    void
    Reset ()
    {
        m_nCurrentState = IDLE;
    }

    /*******************************************************/
    void
    BeginSwitch (Context ctx)
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

    /*******************************************************/
    uint32_t
    GetDestPlayer ()
    {
        if (m_bDestPlayerSet)
            return *MissionRandomizer_GlobalsManager::PP_CURRENT_PED;
        return uint32_t(m_Context.destPlayer);
    }
    
    /*******************************************************/
    bool
    HasDescentFinished ()
    {
        return !"IS_PLAYER_SWITCH_IN_PROGRESS"_n();
            return true;
    }

    /*******************************************************/
    bool
    Process (scrProgram *, scrThreadContext *)
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
                    m_nCurrentState = IDLE;
                break;
            }

        return false;
    }
};
