#include <common/logger.hh>
#include <exceptions/exceptions_Mgr.hh>
#include <common/common.hh>
#include <common/config.hh>

#include <Natives.hh>
#include <CutSceneManager.hh>
#include <CTheScripts.hh>
#include <CLoadingScreens.hh>
#include <Utils.hh>
#include <scrThread.hh>
#include <rage.hh>

#include "missions.hh"

#include <map>
#include <random>
#include <cstdint>

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
    static bool
    ProcessComponents (scrProgram *program, scrThreadContext *ctx)
    {
        Components::sm_Globals.Process (ctx, program);
        Components::sm_Order.Process (ctx, program);

        return Components::sm_PlayerSwitcher.Process ()
               && Components::sm_Flow.Process (program, ctx);
    }

    /*******************************************************/
    static eScriptState
    RunThreadHook (uint64_t *stack, uint64_t *globals, scrProgram *program,
                   scrThreadContext *ctx)
    {
        eScriptState state = ctx->m_nState;
        if (ProcessComponents (program, ctx))
            {
                Components::sm_Flow.SetVariables (ctx);
                state = scrThread_Runff6 (stack, globals, program, ctx);
                Components::sm_Flow.ClearVariables (ctx);
            }

        ProcessComponents (program, ctx);

        return state;
    }

    /*******************************************************/
    static void
    RequestCutsceneHook (scrThread::Info *info)
    {
        Rainbomizer::Logger::LogMessage (
            "LogRequestCutscene: [%s:%03x] - %s",
            scrThread::GetActiveThread ()->m_szScriptName,
            scrThread::GetActiveThread ()->m_Context.m_nIp,
            info->GetArg<char *> (0));
    }

public:
    /*******************************************************/
    MissionRandomizer ()
    {
        if (!ConfigManager::ReadConfig (
                "MissionRandomizer", std::pair ("Seed", &Config ().Seed),
                std::pair ("ForceSeedOnSaves", &Config ().ForceSeedOnSaves),
                std::pair ("ForcedMission", &Config ().ForcedMission),
                std::pair ("LogMissionOrder", &Config ().LogMissionOrder)))
            return;

        InitialiseAllComponents ();

        Components::sm_Globals.Initialise();
        
        RegisterHook ("8d 15 ? ? ? ? ? 8b c0 e8 ? ? ? ? ? 85 ff ? 89 1d", 9,
                      scrThread_Runff6, RunThreadHook);

        Rainbomizer::Common::AddInitCallback (
            [] (bool) { Components::sm_Flow.Reset (); });

        NativeCallbackMgr::InitCallback<"REQUEST_CUTSCENE"_joaat,
                                        RequestCutsceneHook, true> ();
        NativeCallbackMgr::InitCallback<"_REQUEST_CUTSCENE_EX"_joaat,
                                        RequestCutsceneHook, true> ();
    }
} missions;
