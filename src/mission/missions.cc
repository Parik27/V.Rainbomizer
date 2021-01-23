#include <Utils.hh>
#include <cstdint>
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
#include <random>

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
    uint64_t field_0x48[2];
    union
    {
        uint32_t nTriggerFlags;
        struct
        {
            bool Michael : 1;
            bool Franklin : 1;
            bool Trevor : 1;
        } FLAG_TriggerFlags;
    };

    uint8_t field_0x50[180];
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
            bool        LogMissionOrder;

            std::string ForcedMission;

        } m_Config;

        return m_Config;
    }

    struct MissionAssociation
    {
        uint32_t           RandomizedHash;
        uint32_t           OriginalHash;
        std::string        RandomizedName;
        std::string        OriginalName;
        MissionDefinition *RandomizedDefinition;
        MissionDefinition *OriginalDefinition;
    };

    inline static struct
    {
        bool *FLAG_PLAYER_PED_INTRODUCED_T;
        bool *FLAG_PLAYER_PED_INTRODUCED_F;
        bool *FLAG_PLAYER_PED_INTRODUCED_M;
        bool *SP0_AVAILABLE;
        bool *SP1_AVAILABLE;
        bool *SP2_AVAILABLE;
        int* PP_CURRENT_PED;
    } m_Globals;

    inline static uint32_t g_CurrentMission         = 0;
    inline static uint64_t g_PreviousCurrentMission = 0;

    // contains information about the new mission
    inline static std::map<uint32_t, MissionAssociation> m_MissionAssociations;

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
                        assc.OriginalName       = missions[i].sMissionThread;
                        assc.OriginalDefinition = &missions[i];
                    }
                else
                    {
                        assc.RandomizedHash       = missions[i].nThreadHash;
                        assc.RandomizedName       = missions[i].sMissionThread;
                        assc.RandomizedDefinition = &missions[i];

                        if (Config ().LogMissionOrder)
                            {
                                Rainbomizer::Logger::LogMessage (
                                    "%s (%x) -> %s (%x)",
                                    assc.OriginalName.c_str (),
                                    assc.OriginalHash,
                                    assc.RandomizedName.c_str (),
                                    assc.RandomizedHash);
                            }
                    }
            }
    }

    /*******************************************************/
    static void
    RandomizeMissions (MissionDefinition *missions, uint32_t totalMissions)
    {
        InitialiseAssociations (missions, totalMissions, true);
        uint32_t seed
            = static_cast<uint32_t> (std::hash<std::string>{}(Config ().Seed));

        std::mt19937                                engine{seed};
        std::uniform_int_distribution<unsigned int> dist (0, totalMissions - 1);

        for (uint32_t i = 0; i < totalMissions; i++)
            {
                // Forced Mission Enabled
                if (Config ().ForcedMission.size () > 0)
                    {
                        strncpy (missions[i].sMissionThread,
                                 Config ().ForcedMission.c_str (), 24);

                        missions[i].nThreadHash
                            = rage::atStringHash (Config ().ForcedMission);

                        continue;
                    }

                // Forced Mission Disabled
                MissionDefinition &newMission = missions[dist (engine)];

                std::swap (missions[i].sMissionThread,
                           newMission.sMissionThread);
                std::swap (missions[i].nThreadHash, newMission.nThreadHash);
            }

        InitialiseAssociations (missions, totalMissions, false);
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
                           scrThread::GetGlobal<uint32_t> (globalOffset));
    }

    /*******************************************************/
    static bool
    HandleOnMissionStartCommands (uint32_t    originalMission,
                                  uint32_t    randomizedMission,
                                  scrProgram *program)
    {

        *m_Globals.FLAG_PLAYER_PED_INTRODUCED_M = true;
        *m_Globals.FLAG_PLAYER_PED_INTRODUCED_T = true;
        *m_Globals.FLAG_PLAYER_PED_INTRODUCED_F = true;
        *m_Globals.SP0_AVAILABLE                = true;
        *m_Globals.SP1_AVAILABLE                = true;
        *m_Globals.SP2_AVAILABLE                = true;

        // Most missions need this to not get softlocked.
        if ("IS_CUTSCENE_ACTIVE"_n())
            {
                "REMOVE_CUTSCENE"_n();
                return false;
            }

        switch (originalMission)
            {
            // Prologue - to prevent infinite loading.
            case "prologue1"_joaat:
                case "armenian1"_joaat: {
                    "SHUTDOWN_LOADING_SCREEN"_n();
                    "DO_SCREEN_FADE_IN"_n(0);

                    break;
                }
            }

        switch (randomizedMission)
            {
                case "franklin0"_joaat: {
                    *m_Globals.FLAG_PLAYER_PED_INTRODUCED_F = true;
                    *m_Globals.SP2_AVAILABLE                = true;
                    break;
                }
                case "armenian2"_joaat: {
                    *m_Globals.PP_CURRENT_PED = 1;
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
                            "g_CurrentMission changed: %u to %u",
                            g_PreviousCurrentMission, GetCurrentMission ());

                        g_PreviousCurrentMission = GetCurrentMission ();
                    }
            }

        if (ctx->m_nIp == 0 && g_CurrentMission && GetCurrentMission () != -1u)
            {
                try
                    {
                        auto &assoc
                            = m_MissionAssociations.at (GetCurrentMission ());
                        if (ctx->m_nScriptHash == assoc.RandomizedHash)
                            {
                                return HandleOnMissionStartCommands (
                                    assoc.OriginalHash, assoc.RandomizedHash,
                                    program);
                            }
                    }
                catch (...)
                    {
                    }
            }

        return true;
    }

    /*******************************************************/
    static uint64_t
    GetCurrentMission ()
    {
        if (scrThread::GetGlobals ())
            return scrThread::GetGlobal<uint64_t> (g_CurrentMission);

        return -1u;
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
                Rainbomizer::Logger::LogMessage ("g_CurrentMission = %d",
                                                 g_CurrentMission);
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

    /*******************************************************/
    template<typename T>
    static void
    RegisterFieldHook (scrThread::Info *info)
    {
        using namespace std::string_literals;

        const char *fieldName = info->GetArg<const char *> (1);
        T *         ptr       = info->GetArg<T *> (0);

#define ADD_SAVE_DATA_GLOBAL(global)                                           \
    if (fieldName == #global##s)                                               \
    m_Globals.global = ptr

        if constexpr (std::is_same_v<T, bool>)
            {
                ADD_SAVE_DATA_GLOBAL (FLAG_PLAYER_PED_INTRODUCED_T);
                ADD_SAVE_DATA_GLOBAL (FLAG_PLAYER_PED_INTRODUCED_M);
                ADD_SAVE_DATA_GLOBAL (FLAG_PLAYER_PED_INTRODUCED_F);
                ADD_SAVE_DATA_GLOBAL (SP0_AVAILABLE);
                ADD_SAVE_DATA_GLOBAL (SP1_AVAILABLE);
                ADD_SAVE_DATA_GLOBAL (SP2_AVAILABLE);
            }
        else if constexpr (std::is_same_v<T, int>)
            {
                ADD_SAVE_DATA_GLOBAL (PP_CURRENT_PED);
            }

#undef ADD_SAVE_DATA_GLOBAL
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

        RegisterHook ("8d 15 ? ? ? ? ? 8b c0 e8 ? ? ? ? ? 85 ff ? 89 1d", 9,
                      scrThread_Runff6, RunThreadHook);

        NativeCallbackMgr::InitCallback<"REGISTER_BOOL_TO_SAVE"_joaat,
                                        RegisterFieldHook<bool>, false> ();
        NativeCallbackMgr::InitCallback<"REGISTER_ENUM_TO_SAVE"_joaat,
                                        RegisterFieldHook<int>, false> ();
    }
} missions;
