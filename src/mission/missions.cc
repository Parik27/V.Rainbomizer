#include <Utils.hh>
#include <scrThread.hh>
#include <rage.hh>
#include <common/logger.hh>
#include <exceptions/exceptions_Mgr.hh>
#include "missions_YscUtils.hh"
#include <Natives.hh>
#include <CutSceneManager.hh>
#include <common/config.hh>

/*
  Relevant Globals:

  g_14133: g_MissionTriggers
  g_14133[size=34] = name; (offset 0)
  offset 6 = hash
  offset 8 = gxt entry
  offset F = stack size

 */

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

static_assert(sizeof(MissionDefinition) == 0x22 * 8);

class MissionRandomizer
{

    inline static struct Config
    {
        std::string Seed;
        bool ForceSeedOnSaves;

        std::string ForcedMission;
        
    } m_Config;
    
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
    AdjustMissionTriggerCode (scrProgram *program)
    {
        YscUtils util (program);

        
        util.FindCodePattern ("38 02 42 08 61 5d ae 08 38 02 42 09 55 60 06",
                              [] (hook::pattern_match match) {
                                  *match.get<uint32_t> (-4) = 0x08569e61;
                              });

        util.FindCodePattern ("61 89 b3 08 38 02 42 0a 61 75 b1 08 38 02",
                              [] (hook::pattern_match match) {
                                  *match.get<uint32_t> (-4) = 0x08586661;
                              });

        util.FindString ("ARMENIAN_2_INT", [] (char *str) {
            memcpy (str, "ARMENIAN_3_INT", strlen ("ARMENIAN_3_INT"));
        });

        
        static const uint8_t SetupCB[]
            = {0x2d, 0x00, 0x02, 0x00, 0x00, 0x6f, 0x2e, 0x00, 0x01};

        util.FindCodePattern (
            "2d 00 03 00 ? 2c ? 00 74 2c ? 00 53 06 56 82 02 3b ef ",
            [] (hook::pattern_match match) {
                memcpy (match.get<void> (0), SetupCB, sizeof (SetupCB));
            });

        //61 3b b8 08 38 02 42 05 61 80 b7 08 38 02
        // util.FindCodePattern ("61 3b b8 08 38 02 42 05 61 80 b7 08 38 02",
        //                       [&] (hook::pattern_match match) {
        //                           *match.get<uint32_t> (0)
        //                               = util.GetCodeOffset ((uint8_t *) newPage)
        //                                     >> 8
        //                                 & 0x61;
        //                       });
    }

    /*******************************************************/
    static void
    RandomizeMissions (MissionDefinition* missions, int totalMissions)
    {
        for (int i = 0; i < totalMissions; i++)
            {
                // Forced Mission Enabled
                if (m_Config.ForcedMission.size () > 0)
                    {
                        strncpy (missions[i].sMissionThread,
                                 m_Config.ForcedMission.c_str (), 24);
                        
                        missions[i].nThreadHash = rage::atStringHash (
                            m_Config.ForcedMission.c_str ());

                        continue;
                    }

                // Forced Mission Disabled
                MissionDefinition &newMission
                    = missions[RandomInt (totalMissions - 1)];

                std::swap (missions[i].sMissionThread,
                           newMission.sMissionThread);
                std::swap (missions[i].nThreadHash, newMission.nThreadHash);
            }
    }

    /*******************************************************/
    static void
    RandomizeMissions (scrProgram *program)
    {
        YscUtils utils (program);
        uint32_t globalOffset = 0;

        Rainbomizer::Logger::LogMessage ("%x", program->m_nScriptHash);

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
    static void
    ProcessMissionFlow (scrProgram *program, scrThreadContext *ctx)
    {
    }

    /*******************************************************/
    static eScriptState
    RunThreadHook (uint64_t *stack, uint64_t *globals, scrProgram *program,
                   scrThreadContext *ctx)
    {
        ProcessMissionFlow (program, ctx);

        eScriptState state = scrThread_Runff6 (stack, globals, program, ctx);

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
            case "martin1"_joaat:
            case "carsteal1"_joaat:
                MakeMissionThinkItsAReplay (program);
                break;

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
                "MissionRandomizer", std::pair ("Seed", &m_Config.Seed),
                std::pair ("ForceSeedOnSaves", &m_Config.ForceSeedOnSaves),
                std::pair ("ForcedMission", &m_Config.ForcedMission)))
            return;

        InitialiseAllComponents ();

        RegisterHook ("8d ? ? 98 00 00 00 e8 ? ? ? ? 83 38 ff 0f 84", 7,
                      fwAssetStore__GetIndexByNamede5b, LogRequestCutscene);

        RegisterHook ("8d 15 ? ? ? ? ? 8b c0 e8 ? ? ? ? ? 85 ff ? 89 1d", 9, scrThread_Runff6, RunThreadHook);
        
        RegisterHook<true> ("8b c8 8b d3 ? 8b 5c ? ? ? 83 c4 20 5f e9", 14,
                            CutSceneManager_StartCutscene, LogStartCutscene);

        RegisterHook ("e8 ? ? ? ? ? 8b c7 ? 8b c8 8b d3 e8", 13,
                      CutSceneManager_StartCutscene, LogStartCutscene);

        RegisterHook ("8b cb e8 ? ? ? ? 8b 43 70 ? 03 c4 a9 00 c0 ff ff", 2,
                      scrProgram_InitNativeTablese188_, ApplyCodeChanges);
    }
} missions;
