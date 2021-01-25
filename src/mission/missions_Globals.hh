#pragma once

#include "mission/missions_YscUtils.hh"

#include <cstdint>

#include <scrThread.hh>
#include <CTheScripts.hh>

#include <common/logger.hh>

/*******************************************************/
/* Global Types */
/*******************************************************/
enum class ePlayerIndex : uint32_t
{
    PLAYER_MICHAEL = 0,
    PLAYER_FRANKLIN,
    PLAYER_TREVOR,
    PLAYER_MULTIPLAYER,
    PLAYER_UNKNOWN
};

/*******************************************************/
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

/*******************************************************/
template <typename T> struct GlobalArrayWrapper
{
    uint64_t nSize = 0;
    T *      Data = nullptr;
};

/*******************************************************/
/*******************************************************/
class MissionRandomizer_GlobalsManager
{
public:
    GlobalArrayWrapper<MissionDefinition> g_Missions;

    // Mission Flow Flags
    inline static bool *FLAG_PLAYER_PED_INTRODUCED_T = nullptr;
    inline static bool *FLAG_PLAYER_PED_INTRODUCED_F = nullptr;
    inline static bool *FLAG_PLAYER_PED_INTRODUCED_M = nullptr;

    // PP_INFO.AVAILABLE
    inline static bool *SP0_AVAILABLE = nullptr;
    inline static bool *SP1_AVAILABLE = nullptr;
    inline static bool *SP2_AVAILABLE = nullptr;

    // PP_INFO.PP_CURRENT_PED
    inline static int *PP_CURRENT_PED = nullptr;

    uint32_t g_CurrentMission = 0;

    uint32_t
    GetCurrentMission ()
    {
        if (scrThread::GetGlobals () && g_CurrentMission)
            return scrThread::GetGlobal<uint32_t> (g_CurrentMission);

        return -1u;
    }

    /*******************************************************/
    template <typename T>
    static void
    RegisterFieldHook (scrThread::Info *info)
    {
        using namespace std::string_literals;

        const char *fieldName = info->GetArg<const char *> (1);
        T *         ptr       = info->GetArg<T *> (0);

#define ADD_SAVE_DATA_GLOBAL(global)                                           \
    if (fieldName == #global##s)                                               \
        global = ptr

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

    /*******************************************************/
    void
    Init_gCurrentMission (scrThreadContext *ctx, scrProgram *program)
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
    }

    /*******************************************************/
    void
    Init_gMissions (scrThreadContext *ctx, scrProgram *program)
    {
        if (program->m_nScriptHash == "standard_global_init"_joaat
            && ctx->m_nState == eScriptState::KILLED)
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
                            "Failed to initialise Mission Randomizer - Cannot "
                            "find offset for gMissions");

                        g_Missions.nSize = 0;
                        g_Missions.Data  = nullptr;

                        return;
                    }

                g_Missions.nSize = scrThread::GetGlobal (globalOffset);
                g_Missions.Data  = &scrThread::GetGlobal<MissionDefinition> (
                    globalOffset + 1);
            }
    }

    /*******************************************************/
    void
    Process (scrThreadContext *ctx, scrProgram *program)
    {
        Init_gCurrentMission (ctx, program);
        Init_gMissions (ctx, program);
    }

    /*******************************************************/
    void
    Initialise ()
    {
        NativeCallbackMgr::InitCallback<"REGISTER_BOOL_TO_SAVE"_joaat,
                                        RegisterFieldHook<bool>, false> ();
        NativeCallbackMgr::InitCallback<"REGISTER_ENUM_TO_SAVE"_joaat,
                                        RegisterFieldHook<int>, false> ();
    }
};
