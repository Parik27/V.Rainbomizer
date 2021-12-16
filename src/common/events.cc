#include "events.hh"
#include "logger.hh"
#include "scrThread.hh"

#include <Utils.hh>
#include <cstdint>

#include <CTheScripts.hh>

// Required for MR::sm_Globals
#include "mission/missions.hh"

using namespace NativeLiterals;

class gameSkeleton;

namespace Rainbomizer {
class EventTriggerer
{
    using MR = MissionRandomizer_Components;

    // Mission Callbacks Related
    inline static uint32_t nCurrMissionHash            = -1;
    inline static bool     bCurrMissionRepeating       = false;
    inline static bool     bTriggerMissionEndNextFrame = false;

    // Fade Callbacks Related
    inline static bool bScreenFadedOut = true;

    /*******************************************************/
    template <auto &gameSkeleton__Init>
    static void
    ProcessInitCallbacks (gameSkeleton *skelly, uint32_t mode)
    {
        gameSkeleton__Init (skelly, mode);
        Logger::LogMessage ("rage::gameSkeleton::Init(%s)",
                            mode == 4 ? "AFTER_MAP_LOAD" : "INIT_SESSION");

        Events ().OnInit (mode != 4);
    }

    /*******************************************************/
    static void
    ProcessMissionCallbacks (scrThreadContext *ctx)
    {
        static uint32_t nPrevCurrMission = -1;

        /* Only process if necessary */
        if (!Events ().OnMissionStart.m_Callbacks.size ()
            && !Events ().OnMissionEnd.m_Callbacks.size ())
            return;

        if (!MR::sm_Globals.g_CurrentMission.Init ()
            || !MR::sm_Globals.g_Missions.Init ())
            return;

        if (MR::sm_Globals.g_CurrentMission != nPrevCurrMission)
            {
                auto g_Missions  = MR::sm_Globals.g_Missions.Get ();
                nPrevCurrMission = MR::sm_Globals.g_CurrentMission;

                nCurrMissionHash
                    = g_Missions->Data[nPrevCurrMission].nThreadHash;

                bCurrMissionRepeating = false;
            }

        if (ctx->m_nIp == 0 && ctx->m_nScriptHash == nCurrMissionHash)
            Events ().OnMissionStart (nCurrMissionHash, bCurrMissionRepeating);

        if (bTriggerMissionEndNextFrame)
            {
                nCurrMissionHash      = -1u;
                bCurrMissionRepeating = true;
                Events ().OnMissionEnd ();
            }
    }

    /*******************************************************/
    static void
    ProcessFadeCallbacks (scrThreadContext *ctx)
    {
        if (ctx->m_nScriptHash == "main"_joaat)
            {
                if (std::exchange (bScreenFadedOut,
                                   "IS_SCREEN_FADED_OUT"_n()
                                       || "IS_PLAYER_SWITCH_IN_PROGRESS"_n())
                        != bScreenFadedOut
                    && bScreenFadedOut)
                    Events ().OnFade ();
            }
    }

    /*******************************************************/
    static void
    ProcessScriptStartCallbacks (scrThreadContext *ctx)
    {
        if (ctx->m_nIp == 0)
            Events ().OnScriptStart (ctx->m_nScriptHash);
    }

    /*******************************************************/
    template <auto &scrThread__Run>
    static eScriptState
    RunThreadHook (uint64_t *stack, uint64_t *globals, scrProgram *program,
                   scrThreadContext *ctx)
    {
        ProcessMissionCallbacks (ctx);
        ProcessFadeCallbacks (ctx);
        ProcessScriptStartCallbacks (ctx);
        Events ().OnRunThread (stack, globals, program, ctx);
        auto ret = scrThread__Run (stack, globals, program, ctx);

        if (ret == eScriptState::KILLED
            && ctx->m_nScriptHash == nCurrMissionHash)
            bTriggerMissionEndNextFrame = true;

        return ret;
    }

public:
    EventTriggerer ()
    {
        REGISTER_HOOK ("e8 ? ? ? ? e8 ? ? ? ? ? 8d 0d ? ? ? ? ba 04 00 00 00",
                       22, ProcessInitCallbacks, void, gameSkeleton *,
                       uint32_t);

        REGISTER_HOOK (
            "? 8d 0d ? ? ? ? ba 08 00 00 00 e8 ? ? ? ? c6 05 ? ? ? ? 01 ", 12,
            ProcessInitCallbacks, void, gameSkeleton *, uint32_t);

        REGISTER_HOOK ("8d 15 ? ? ? ? ? 8b c0 e8 ? ? ? ? ? 85 ff ? 89 1d", 9,
                       RunThreadHook, eScriptState, uint64_t *, uint64_t *,
                       scrProgram *, scrThreadContext *);
    }

} g_EventTriggerer;
}; // namespace Rainbomizer
