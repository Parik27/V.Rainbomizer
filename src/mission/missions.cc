#include <common/logger.hh>
#include <exceptions/exceptions_Mgr.hh>
#include <common/events.hh>
#include <common/config.hh>

#include <Natives.hh>
#include <CutSceneManager.hh>
#include <CTheScripts.hh>
#include <Utils.hh>
#include <scrThread.hh>
#include <rage.hh>

#include "mission/missions_Globals.hh"
#include "missions.hh"

#include <map>
#include <random>
#include <cstdint>

#ifdef ENABLE_DEBUG_MENU
#include <debug/base.hh>
#endif

using namespace NativeLiterals;

class fwAssetStore;
class CutSceneManager;

eScriptState (*scrThread_Runff6) (uint64_t *, uint64_t *, scrProgram *,
                                  scrThreadContext *);

class MissionRandomizer
{
    using Components = MissionRandomizer_Components;

    static auto &
    Config ()
    {
        return Components::Config ();
    }

    /*******************************************************/
    static eScriptState
    RunThreadHook (uint64_t *stack, uint64_t *globals, scrProgram *program,
                   scrThreadContext *ctx)
    {
        eScriptState state = ctx->m_nState;
        if (Components::Process (program, ctx)
            || Components::IsScriptAllowedOnPause (ctx->m_nScriptHash))
            {
                Components::sm_Flow.SetVariables (ctx);
                state = scrThread_Runff6 (stack, globals, program, ctx);
                Components::sm_Flow.ClearVariables (ctx);
            }

        Components::Process (program, ctx);

        return state;
    }

    /*******************************************************/
    static void
    ChangeToPlayer (ePlayerIndex idx)
    {
        MissionRandomizer_PlayerSwitcher::Context ctx;
        ctx.destPlayer     = idx;
        ctx.noSetPos       = true;
        ctx.transitionType = ctx.NO_TRANSITION;
        Components::sm_PlayerSwitcher.BeginSwitch (ctx);
    }

public:
    /*******************************************************/
    MissionRandomizer ()
    {
        if (!ConfigManager::ReadConfig (
                "MissionRandomizer", std::pair ("Seed", &Config ().Seed),
                std::pair ("ForceSeedOnSaves", &Config ().ForceSeedOnSaves),
                std::pair ("ForcedMission", &Config ().ForcedMission),
                std::pair ("EnableFastSkips", &Config ().EnableFastSkips),
                std::pair ("LogMissionOrder", &Config ().LogMissionOrder)))
            return;

        InitialiseAllComponents ();

        Components::sm_Globals.Initialise ();
        Components::sm_CodeFixes.Initialise ();

        RegisterHook ("8d 15 ? ? ? ? ? 8b c0 e8 ? ? ? ? ? 85 ff ? 89 1d", 9,
                      scrThread_Runff6, RunThreadHook);

        Rainbomizer::Events ().OnInit +=
            [] (bool) { Components::sm_Flow.Reset (); };

#ifdef ENABLE_DEBUG_MENU
        DebugInterfaceManager::AddAction (
            "Change to Franklin",
            std::bind (ChangeToPlayer, ePlayerIndex::PLAYER_FRANKLIN));
        DebugInterfaceManager::AddAction (
            "Change to Michael",
            std::bind (ChangeToPlayer, ePlayerIndex::PLAYER_MICHAEL));
        DebugInterfaceManager::AddAction (
            "Change to Trevor",
            std::bind (ChangeToPlayer, ePlayerIndex::PLAYER_TREVOR));
#endif
    }
} missions;
