#include <Utils.hh>
#include <scrThread.hh>
#include <rage.hh>
#include <common/logger.hh>
#include <exceptions/exceptions_Mgr.hh>
#include "missions_YscUtils.hh"
#include <Natives.hh>
#include <CutSceneManager.hh>
#include <common/config.hh>
#include <CTheScripts.hh>
#include <CLoadingScreens.hh>
#include <common/common.hh>
#include <map>

/*
  Relevant Globals:

  g_14133: g_MissionTriggers
  g_14133[size=34] = name; (offset 0)
  offset 6 = hash
  offset 8 = gxt entry
  offset F = stack size

 */

using namespace NativeLiterals;

class fwAssetStore;
class CutSceneManager;

void *(*fwAssetStore__GetIndexByNamede5b) (fwAssetStore *, uint32_t *,
                                           char const *);
void (*CutSceneManager_StartCutscene) (CutSceneManager *, uint32_t, uint32_t);
eScriptState (*scrThread_Runff6) (uint64_t *, uint64_t *, scrProgram *,
                                  scrThreadContext *);
inline bool (*scrProgram_InitNativeTablese188_) (scrProgram *);

struct MissionDefinition
{
    char     sMissionThread[24];
    uint8_t  field_0x18[24];
    uint32_t nThreadHash;
    uint8_t  field_0x34[12];
    char     sMissionGxtEntry[8];
    uint8_t  field_0x48[200];
};

static_assert (sizeof (MissionDefinition) == 0x22 * 8);

class MissionRandomizer
{
    static auto &
    Config ()
    {
        static struct Config
        {
            std::string Seed;
            bool        ForceSeedOnSaves;

            std::string ForcedMission;

        } m_Config;

        return m_Config;
    }

    struct MissionAssociation
    {
        uint32_t           RandomizedHash;
        uint32_t           OriginalHash;
        MissionDefinition *RandomizedDefinition;
        MissionDefinition *OriginalDefinition;
    };

    inline static int32_t g_CurrentMission = 0;
    inline static int64_t g_PreviousCurrentMission = 0;
    
    // contains information about the new mission
    inline static std::map<uint32_t, MissionAssociation> m_MissionAssociations;

    /*******************************************************/
    static void
    MakeMissionThinkItsAReplay (scrProgram *program)
    {
        YscUtils util (program);

        const static uint8_t instr[] = {
            0x6f,            // PUSH_CONST_1
            0x2e, 0x01, 0x01 // LEAVE 0x1, 0x1
        };

        util.FindCodePattern ("38 00 06 2a 56 ? ? 28 7d d3 48 e5",
                              [] (hook::pattern_match match) {
                                  memcpy (match.get<void *> (0), instr,
                                          sizeof (instr));
                              });
    }

    /*******************************************************/
    static void
    InitialiseAssociations (MissionDefinition *missions, int totalMissions,
                            bool init)
    {
        if (init)
            m_MissionAssociations.clear ();

        for (int i = 0; i < totalMissions; i++)
            {
                MissionAssociation &assc = m_MissionAssociations[i];
                if (init)
                    {
                        assc.OriginalHash       = missions[i].nThreadHash;
                        assc.OriginalDefinition = &missions[i];
                    }
                else
                    {
                        assc.RandomizedHash       = missions[i].nThreadHash;
                        assc.RandomizedDefinition = &missions[i];
                    }
            }
    }

    /*******************************************************/
    static void
    RandomizeMissions (MissionDefinition *missions, int totalMissions)
    {
        InitialiseAssociations(missions, totalMissions, true);
        
        for (int i = 0; i < totalMissions; i++)
            {
                // Forced Mission Enabled
                if (Config ().ForcedMission.size () > 0)
                    {
                        strncpy (missions[i].sMissionThread,
                                 Config ().ForcedMission.c_str (), 24);

                        missions[i].nThreadHash = rage::atStringHash (
                            Config ().ForcedMission.c_str ());

                        continue;
                    }

                // Forced Mission Disabled
                MissionDefinition &newMission
                    = missions[RandomInt (totalMissions - 1)];

                std::swap (missions[i].sMissionThread,
                           newMission.sMissionThread);
                std::swap (missions[i].nThreadHash, newMission.nThreadHash);
            }

        InitialiseAssociations(missions, totalMissions, false);
    }

    /*******************************************************/
    static void
    RandomizeMissions (scrProgram *program)
    {
        YscUtils utils (program);
        uint32_t globalOffset = 0;

        // Find offset to gMissions (Not original name)
        utils.FindCodePattern (
            "2d 09 0b 00 ? 38 01 38 00 5e ? ? ? 34 22 ? ? ? ? ?",
            [&] (hook::pattern_match m) {
                // GLOBAL_U24 <imm24>
                globalOffset = *m.get<uint32_t> (10) & 0xFFFFFF;
            });

        if (globalOffset == 0)
            {
                Rainbomizer::Logger::LogMessage (
                    "Failed to initialise Mission Randomizer - Cannot find "
                    "pattern "
                    "for gMissions (not original name)");
                return;
            }

        RandomizeMissions (&scrThread::GetGlobal<MissionDefinition> (
                               globalOffset + 1),
                           scrThread::GetGlobal<uint64_t> (globalOffset));
    }

    /*******************************************************/
    static void *
    LogRequestCutscene (fwAssetStore *store, uint32_t *outId, char const *name)
    {
        Rainbomizer::Logger::LogMessage (
            "LogRequestCutscene: [%s:%03x] - %s",
            scrThread::GetActiveThread ()->m_szScriptName,
            scrThread::GetActiveThread ()->m_Context.m_nIp, name);

        return fwAssetStore__GetIndexByNamede5b (store, outId, name);
    }

    /*******************************************************/
    static void
    LogStartCutscene (CutSceneManager *cuts, uint32_t thId, uint32_t cutId)
    {
        Rainbomizer::Logger::LogMessage (
            "LogStartCutscene: [%s:%03x] - %d",
            scrThread::GetActiveThread ()->m_szScriptName,
            scrThread::GetActiveThread ()->m_Context.m_nIp, cutId);

        CutSceneManager_StartCutscene (cuts, thId, cutId);
    }

    /*******************************************************/
    static bool
    HandleOnMissionStartCommands (uint32_t originalMission,
                                  uint32_t randomizedMission)
    {
        // Most missions need this to not get softlocked.
        if ("IS_CUTSCENE_ACTIVE"_n())
            {
                "REMOVE_CUTSCENE"_n();
                return false;
            }

        switch (originalMission)
            {
                // Prologue - to prevent infinite loading.
                case "prologue1"_joaat: {
                    "SHUTDOWN_LOADING_SCREEN"_n();
                    "DO_SCREEN_FADE_IN"_n(0);
                    
                    break;
                }

                // Armenian1 - player frozen at the start
                case "armenian1"_joaat: {
                    "DO_SCREEN_FADE_IN"_n(500);
                    break;
                }
            }

        return true;
    }

    /*******************************************************/
    static bool
    ProcessMissionFlow (scrProgram *program, scrThreadContext *ctx)
    {
        if (g_CurrentMission)
            {
                if (GetCurrentMission () != g_PreviousCurrentMission)
                    {
                        Rainbomizer::Logger::LogMessage (
                            "g_CurrentMission changed: %d to %d",
                            g_PreviousCurrentMission, GetCurrentMission ());

                        g_PreviousCurrentMission = GetCurrentMission ();
                    }
            }

        if (ctx->m_nIp == 0 && g_CurrentMission && GetCurrentMission () != -1)
            {
                try
                    {
                        auto &assoc
                            = m_MissionAssociations.at (GetCurrentMission ());
                        if (ctx->m_nScriptHash == assoc.RandomizedHash)
                            {
                                return HandleOnMissionStartCommands (
                                    assoc.OriginalHash, assoc.RandomizedHash);
                            }
                    }
                catch (...)
                    {
                    }
            }

        return true;
    }

    /*******************************************************/
    static int64_t
    GetCurrentMission ()
    {
        if (scrThread::GetGlobals())
            return scrThread::GetGlobal<uint64_t>(g_CurrentMission);

        return -1;
    }
    
    /*******************************************************/
    static eScriptState
    RunThreadHook (uint64_t *stack, uint64_t *globals, scrProgram *program,
                   scrThreadContext *ctx)
    {
        if (ctx->m_nIp == 0
            && program->m_nScriptHash == "flow_controller"_joaat)
            {
                YscUtils utils (program);

                // Find offset to g_CurrentMission (Not original name)
                utils.FindCodePattern ("2c 04 ? ? 38 02 60 ? ? ? 2e 05 00",
                                       [&] (hook::pattern_match m) {
                                           // GLOBAL_U24 <imm24>
                                           g_CurrentMission
                                               = *m.get<uint32_t> (7)
                                                 & 0xFFFFFF;
                                       });
                Rainbomizer::Logger::LogMessage("g_CurrentMission = %d", g_CurrentMission);
            }

        eScriptState state = ctx->m_nState;
        if (ProcessMissionFlow (program, ctx))
            state = scrThread_Runff6 (stack, globals, program, ctx);

        // standard_global_init initialises the gMissions (not actual name)
        // structure that stores the thread name/gxt entry etc.
        if (program->m_nScriptHash == "standard_global_init"_joaat
            && state == eScriptState::KILLED)
            {
                RandomizeMissions (program);
            }
        return state;
    }

    /*******************************************************/
    static bool
    ApplyCodeChanges (scrProgram *program)
    {
        switch (program->m_nScriptHash)
            {
            case "flow_controller"_joaat:
                YscUtils utils (program);
                utils.FindCodePattern ("5e ? ? ? 25 08 2c 08 ? ? 55 ? ? ",
                                       [] (hook::pattern_match m) {
                                           *m.get<uint32_t> (6) = 0;
                                       });

                utils.FindCodePattern ("5e ? ? ? 38 03 25 08 2c 0c",
                                       [] (hook::pattern_match m) {
                                           *m.get<uint32_t> (8) = 0;
                                       });

                utils.FindCodePattern ("5e ? ? ? 2c ? ? ? 56 ? ? 6e ",
                                       [] (hook::pattern_match m) {
                                           memset (m.get<void> (), 0, 11);
                                       });
                break;
            }
        
        return scrProgram_InitNativeTablese188_ (program);
    }

public:
    /*******************************************************/
    MissionRandomizer ()
    {
        if (!ConfigManager::ReadConfig (
                "MissionRandomizer", std::pair ("Seed", &Config ().Seed),
                std::pair ("ForceSeedOnSaves", &Config ().ForceSeedOnSaves),
                std::pair ("ForcedMission", &Config ().ForcedMission)))
            return;

        InitialiseAllComponents ();

        RegisterHook ("8d ? ? 98 00 00 00 e8 ? ? ? ? 83 38 ff 0f 84", 7,
                      fwAssetStore__GetIndexByNamede5b, LogRequestCutscene);

        RegisterHook ("8d 15 ? ? ? ? ? 8b c0 e8 ? ? ? ? ? 85 ff ? 89 1d", 9, scrThread_Runff6, RunThreadHook);
        
        RegisterHook<true> ("8b c8 8b d3 ? 8b 5c ? ? ? 83 c4 20 5f e9", 14,
                            CutSceneManager_StartCutscene, LogStartCutscene);

        RegisterHook ("e8 ? ? ? ? ? 8b c7 ? 8b c8 8b d3 e8", 13,
                      CutSceneManager_StartCutscene, LogStartCutscene);
        
        // RegisterHook ("8b cb e8 ? ? ? ? 8b 43 70 ? 03 c4 a9 00 c0 ff ff", 2,
        //               scrProgram_InitNativeTablese188_, ApplyCodeChanges);
    }
} missions;
