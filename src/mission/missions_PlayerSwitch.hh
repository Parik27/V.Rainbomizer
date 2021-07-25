#pragma once

#include "mission/missions_Globals.hh"
#include "rage.hh"
#include "scrThread.hh"

class MissionRandomizer_PlayerSwitcher
{
public:
    struct Context
    {
        ePlayerIndex destPlayer = ePlayerIndex::PLAYER_MICHAEL;

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
        TRANSITION_DESCEND,
        TRANSITION_CLEANUP
    } m_nCurrentState
        = IDLE;

    bool m_bDestPlayerSet = true;

    Context m_Context;

    bool CheckCurrentPlayer (ePlayerIndex index);
    bool SetCurrentPlayer (ePlayerIndex index);

    bool IsPlayerSceneOnGoing ();

    bool DoTransitionBegin ();
    bool DoTransitionProcess ();
    bool DoPlayerSwitch ();
    bool DoTransitionEnd ();
    bool DoTransitionProcessEnd ();
    bool DoTransitionCleanup ();

public:
    /*******************************************************/
    void
    Reset ()
    {
        m_nCurrentState = IDLE;
    }

    void     BeginSwitch (Context ctx);
    uint32_t GetDestPlayer ();
    bool     HasDescentFinished ();

    bool Process (scrProgram *, scrThreadContext *);
};
