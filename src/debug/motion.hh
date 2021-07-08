#include "CPed.hh"
#include "base.hh"
#include <CPed.hh>

class MotionDebugInterface : public DebugInterface
{
    inline static uint32_t nLastMotionState = "motionstate_none"_joaat;

    const char *
    GetMotionState (uint32_t hash)
    {
#define X(state)                                                               \
    case state##_joaat: return state;
        switch (hash)
            {
                X ("motionstate_none")
                X ("motionstate_idle")
                X ("motionstate_walk")
                X ("motionstate_run")
                X ("motionstate_sprint")
                X ("motionstate_crouch_idle")
                X ("motionstate_crouch_walk")
                X ("motionstate_crouch_run")
                X ("motionstate_donothing")
                X ("motionstate_invehicle")
                X ("motionstate_aiming")
                X ("motionstate_diving_idle")
                X ("motionstate_diving_swim")
                X ("motionstate_swimming_treadwater")
                X ("motionstate_dead")
                X ("motionstate_stealth_idle")
                X ("motionstate_stealth_walk")
                X ("motionstate_stealth_run")
                X ("motionstate_parachuting")
                X ("motionstate_actionmode_idle")
                X ("motionstate_actionmode_walk")
                X ("motionstate_actionmode_run")
            }
#undef X
        return "unknown";
    }

    void
    Draw () override
    {
        CPed *player = CPedFactory::Get ()->pPlayer;
        if (!player)
            return;

        ImGui::Text ("Player: %p", player);
        ImGui::Text ("Vehicle: %p", player->GetVehicle ());
        ImGui::Text ("Intelligence: %p", player->GetIntelligence ());
        ImGui::Text ("Motion State: %s (%x)", GetMotionState (nLastMotionState),
                     nLastMotionState);
    }

public:
    static void
    SetLastKnownPlayerMotionState (uint32_t state)
    {
        nLastMotionState = state;
    }

    const char *
    GetName () override
    {
        return "Motion";
    }
} inline g_MotionDebugInterface;
