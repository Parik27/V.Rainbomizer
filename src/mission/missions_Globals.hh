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

enum eFlowFlag
{
    FLAG_H_BOARD_UNDO               = 0,
    FLAG_H_FIN_DOCKS                = 1,
    FLAG_H_FIN_AGENCY               = 2,
    FLAG_H_FIN_FINALE               = 3,
    FLAG_H_FIN_JEWEL                = 4,
    FLAG_H_FIN_RURALBANK            = 5,
    FLAG_H_AGENCY_PRIME_BOARD       = 6,
    FLAG_H_AGENCY_LOAD_B_EXIT       = 7,
    FLAG_H_AGENCY_DO_B_EXIT         = 8,
    FLAG_H_AGENCY_CALLS_COMPLETE    = 9,
    FLAG_H_AGENCY_2_UNLOCKED        = 10,
    FLAG_H_AGENCY_2_AUTOTRIG        = 11,
    FLAG_H_AGENCY_P_2_DONE          = 12,
    FLAG_H_AGENCY_FT_GOT_WANTED     = 13,
    FLAG_H_AGENCY_P_CALLS_DONE      = 14,
    FLAG_H_JEWEL_P2A_R              = 15,
    FLAG_H_JEWEL_PRIME_BOARD        = 16,
    FLAG_H_JEWEL_LOAD_B_EXIT        = 17,
    FLAG_H_JEWEL_DO_B_EXIT          = 18,
    FLAG_H_JEWEL_RUN                = 19,
    FLAG_H_DOCKS_PRIME_BOARD        = 20,
    FLAG_H_DOCKS_LOAD_B_EXIT        = 21,
    FLAG_H_DOCKS_DO_B_EXIT          = 22,
    FLAG_DOCKS_PRE_H_TEXTS_DONE     = 23,
    FLAG_H_FINALE_PRIME_BOARD       = 24,
    FLAG_H_FINALE_LOAD_B_EXIT       = 25,
    FLAG_H_FINALE_DO_B_EXIT         = 26,
    FLAG_H_FINALE_P_GOT_WANTED      = 27,
    FLAG_H_FINALE_PA_CALLS_DONE     = 28,
    FLAG_H_FINALE_PB_CALLS_DONE     = 29,
    FLAG_H_FINALE_PC_EMAIL_DONE     = 30,
    FLAG_H_FINALE_PC_1_STOLEN       = 31,
    FLAG_H_FINALE_PC_2_STOLEN       = 32,
    FLAG_H_FINALE_PC_3_STOLEN       = 33,
    FLAG_H_FINALE_P_E_DONE          = 34,
    FLAG_H_FINALE_P_E_PLACE         = 35,
    FLAG_H_FINALE_2_READY           = 36,
    FLAG_H_RURAL_LOAD_MIKE_WIN      = 37,
    FLAG_H_RURAL_LOAD_TREV_WIN      = 38,
    FLAG_H_RURAL_LOAD_B_EXIT        = 39,
    FLAG_H_RURAL_DO_B_EXIT          = 40,
    FLAG_H_RURAL_PREP_CALL_DONE     = 41,
    FLAG_UNLOCK_LAMAR_1             = 42,
    FLAG_FRANK1_MC_CLIPPED          = 43,
    FLAG_FBI_3_CALLS_DONE           = 44,
    FLAG_FBI_4_PREP_3_COMPLETED     = 45,
    FLAG_FBI_4_UNLOCKED_FROM_P      = 46,
    FLAG_FBI_4_CALLS_DONE           = 47,
    FLAG_ASS_1_UNLOCKED             = 48,
    FLAG_ASS_COMPLETED              = 49,
    FLAG_MICHAEL_4_TEXTS_DONE       = 50,
    FLAG_MARTIN_1_CALLS_DONE        = 51,
    FLAG_ALLOW_RANDOM_EVENTS        = 52,
    FLAG_ALLOW_CINEMA               = 53,
    FLAG_ALLOW_COMEDYCLUB           = 54,
    FLAG_ALLOW_LIVEMUSIC            = 55,
    FLAG_ALLOW_SHOP_ROBBERIES       = 56,
    FLOW_BLOCK_SH_FOR_SAVE          = 57,
    FLAG_T_RESTRICT_COUNTRY         = 58,
    FLAG_T_RESTRICT_CITY            = 59,
    FLOWFLAG_F_FROZEN_POST_PRO      = 60,
    FLOWFLAG_ARM1_CAR_DAMAGED       = 61,
    FLAG_STRETCH_TEXT_SENT          = 62,
    FLAG_CHOP_DOG_UNLOCKED          = 63,
    FLAG_CAR3_INITIALISED           = 64,
    FLAG_MOVIE_STUDIO_OPEN          = 65,
    FLAG_MOVIE_STUDIO_OPEN_FRAN     = 66,
    FLAG_AIR_VEH_PARA_UNLOCKED      = 67,
    FLAG_WATER_VEH_SCUBA_UNLOCKED   = 68,
    FLAG_OFFR_RACE_UNLOCK_ALL       = 69,
    FLAG_RUN_BUILDSITE_AMB_AUD      = 70,
    FLAG_MICHAEL_D_FLYING_LES       = 71,
    FLAG_ARM3_STRIPSWITCH_AVAIL     = 72,
    FLAG_DARTS_YELLOW_JACK_AV       = 73,
    FLAG_RES_AND_RCS_UNLOCKED       = 74,
    FLAG_EXILE1_PICKUPS_UNLOCKED    = 75,
    FLAG_CAR2_KILLED_MULLIGAN       = 76,
    FLAG_FRAN_DONE_ACT_WITH_LAM     = 77,
    FLAG_TREV_DONE_ACT_WITH_LAM     = 78,
    FLOWFLAG_AMANDA_M_EV_SKIPPED    = 79,
    FLOWFLAG_SHRINK_KILLED          = 80,
    FLOWFLAG_BLOCK_F_MISSIONS_FOR_T = 81,
    FLAG_GUNRANGE_TUTORIAL_DONE     = 82,
    FLAG_WILD_PHOTO_UNLOCKED        = 83,
    FLAG_BARRY3_TEXT_RECEIVED       = 84,
    FLAG_BARRY4_TEXT_RECEIVED       = 85,
    FLAG_EPSILON_QUESTIONNAIRE_DONE = 86,
    FLAG_EPSILON_DONATED_500        = 87,
    FLAG_EPSILON_DONATED_5000       = 88,
    FLAG_EPSILON_DONATED_10000      = 89,
    FLAG_EPSILON_CARS_DONE          = 90,
    FLAG_EPSILON_ROBES_BOUGHT       = 91,
    FLAG_EPSILON_ROBES_DONE         = 92,
    FLAG_EPSILON_6_TEXT_RECEIVED    = 93,
    FLAG_EPSILON_DESERT_DONE        = 94,
    FLAG_EPSILON_UNLOCKED_TRACT     = 95,
    FLAG_EXTREME2_TEXT_RECEIVED     = 96,
    FLAG_EXTREME3_TEXT_RECEIVED     = 97,
    FLAG_EXTREME4_BJUMPS_FINISHED   = 98,
    FLAG_HUNTINTG1_TEXT_RECEIVED    = 99,
    FLAG_NIGEL1_EMAIL_RECEIVED      = 100,
    FLAG_NIGEL3_NAPOLI_KILLED       = 101,
    FLAG_PAPARAZZO3_TEXT_RECEIVED   = 102,
    FLAG_TONYA3_TEXT_RECEIVED       = 103,
    FLAG_TONYA4_TEXT_RECEIVED       = 104,
    FLAG_FOR_SALE_SIGNS_DESTROYED   = 105,
    FLAG_LETTER_SCRAPS_DONE         = 106,
    FLAG_SPACESHIP_PARTS_DONE       = 107,
    FLAG_ALL_RAMPAGES_UNLOCKED      = 108,
    FLAG_PURCHASED_MARINA_PROPERTY  = 109,
    FLAG_DIVING_SCRAPS_DONE         = 110,
    FLAG_MIKE_CON_BOUGHT            = 111,
    FLAG_FRANK_CON_BOUGHT           = 112,
    FLAG_TREV_CON_BOUGHT            = 113,
    FLAG_BEV_SENT_WILD_TXT          = 114,
    FLAG_PLAYER_PED_INTRODUCED_M    = 115,
    FLAG_PLAYER_PED_INTRODUCED_F    = 116,
    FLAG_PLAYER_PED_INTRODUCED_T    = 117,
    FLAG_PLAYER_VEH_F_UNLOCK_BIKE   = 118,
    FLAG_PLAYER_VEH_T_UNLOCK_RJAM   = 119,
    FLAG_MIC_HAS_HAGGARD_SUIT       = 120,
    FLAG_MIC_SET_HAGGARD_SUIT       = 121,
    FLAG_MIC_REM_HAGGARD_SUIT       = 122,
    FLOW_MIC_HIDE_BARE_CHEST        = 123,
    FLOW_MIC_PRO_MASK_REMOVED       = 124,
    FLOW_TRV_PRO_MASK_REMOVED       = 125,
    FLAG_F_MOVED_TO_HILLS_APART     = 126,
    FLAG_M_F_ARE_FRIENDS            = 127,
    FLAG_M_A_HAVE_SPLIT             = 128,
    FLAG_M_A_HAVE_FALLEN_OUT        = 129,
    FLAG_M_T_EXILE_STARTED          = 130,
    FLAG_M_T_EXILE_FINISHED         = 131,
    FLAG_ORTEGA_KILLED              = 132,
    FLAG_GAME_100_PERC_COMP         = 133,
    FLAG_FINAL_CHOICE_MADE          = 134,
    FLAG_MICHAEL_KILLED             = 135,
    FLAG_TREVOR_KILLED              = 136,
    FLAG_RESPAWNED_AFTER_FIN        = 137,
    FLAG_PLAYER_HAS_USED_FP_VIEW    = 138,
    FLAG_SENT_PROX_MINE_EMAIL       = 139,
    FLAG_SENT_MISSILE_EMAIL         = 140,
};

enum eMissionFlowControlIntId
{
    INT_H_TRIGGER_HEIST     = 0,
    INT_H_TRIGGER_MISSION   = 1,
    INT_H_BOARD_M_JEWEL     = 2,
    INT_H_BOARD_M_DOCKS     = 3,
    INT_H_BOARD_M_RURAL     = 4,
    INT_H_BOARD_M_AGENCY    = 5,
    INT_H_BOARD_M_FINALE    = 6,
    INT_HEIST_CHOICE_JEWEL  = 7,
    INT_HEIST_CHOICE_DOCKS  = 8,
    INT_HEIST_CHOICE_RURAL  = 9,
    INT_HEIST_CHOICE_AGENCY = 10,
    INT_HEIST_CHOICE_FINALE = 11,
    INT_MISS_CHOICE_FINALE  = 12,
};

enum eHeistApproach
{
    JEWEL_STEALTH      = 1,
    JEWEL_HIGH_IMPACT  = 2,
    DOCKS_BLOW_UP_BOAT = 3,
    DOCKS_DEEP_SEA     = 4,
    RURAL_NO_TANK      = 5,
    AGENCY_FIRETRUCK   = 6,
    AGENCY_HELICOPTER  = 7,
    FINALE_TRAFFCONT   = 8,
    FINALE_HELI        = 9,
};

enum class eCharacter
{
    Michael                  = 0,
    Franklin                 = 1,
    Trevor                   = 2,
    Multiplayer              = 3,
    Conference_199           = 4,
    Conference_191           = 5,
    Conference_E_277         = 6,
    Conference_E_278         = 7,
    Conference_192           = 8,
    Conference_193           = 9,
    Conference_189           = 10,
    Conference_190           = 11,
    Lester                   = 12,
    Lester_Deathwish         = 13,
    Jimmy                    = 14,
    Tracey                   = 15,
    Abigail                  = 16,
    Amanda                   = 17,
    Simeon                   = 18,
    Lamar                    = 19,
    Ron                      = 20,
    Cheng_Jr                 = 21,
    Saeeda                   = 22,
    Steve                    = 23,
    Wade                     = 24,
    Tennis_Coach             = 25,
    Solomon                  = 26,
    Lazlow                   = 27,
    Estate_Agent             = 28,
    Devin                    = 29,
    Dave                     = 30,
    Martin                   = 31,
    Floyd                    = 32,
    Military_Hitcher         = 33,
    Oscar                    = 34,
    Cheng_Sr                 = 35,
    Dr_Friedlander           = 36,
    Stretch_122              = 37,
    Ortega                   = 38,
    O_Neil                   = 39,
    Patricia                 = 40,
    Pegasus                  = 41,
    Lifeinvader              = 42,
    Tanisha                  = 43,
    Denise                   = 44,
    Molly                    = 45,
    Rickie                   = 46,
    Chef                     = 47,
    Blimp                    = 48,
    Barry                    = 49,
    Beverly                  = 50,
    Cris                     = 51,
    Dom                      = 52,
    Hao                      = 53,
    Cletus                   = 54,
    Jimmy_Boston             = 55,
    Joe                      = 56,
    Josef                    = 57,
    Josh                     = 58,
    Manuel                   = 59,
    Marnie                   = 60,
    Mary_Ann                 = 61,
    Maude                    = 62,
    Mrs_Thornhill            = 63,
    Nigel                    = 64,
    Beast                    = 65,
    Ashley                   = 66,
    Andreas                  = 67,
    Dreyfuss                 = 68,
    Omega                    = 69,
    Al_Carter                = 70,
    The_Boss_176             = 71,
    Edgar_Claros             = 72,
    The_Boss_178             = 73,
    Maze_Bank                = 74,
    CELL_173                 = 75,
    CELL_165                 = 76,
    Stretch_172              = 77,
    Corey_Parker             = 78,
    CELL_184                 = 79,
    CELL_185                 = 80,
    CELL_182                 = 81,
    CELL_183                 = 82,
    GTA_Online_Contact       = 83,
    Brucie                   = 84,
    Merryweather             = 85,
    Gerald                   = 86,
    Vanilla_Unicorn          = 87,
    Detonate_Bomb            = 88,
    Mechanic_E_MP0           = 89,
    Julio_Fabrizio           = 90,
    Mors_Mutual_Insurance    = 91,
    Domestic_Girl            = 92,
    Ursula                   = 93,
    Fixed_Car                = 94,
    Antonia                  = 95,
    Los_Santos_Customs_E_209 = 96,
    Ammu_Nation              = 97,
    Social_Club              = 98,
    LS_Tourist_Board         = 99,
    Mechanic_180             = 100,
    Sonar_Collections        = 101,
    Chop                     = 102,
    Tonya                    = 103,
    Juliet                   = 104,
    Nikki                    = 105,
    Chastity                 = 106,
    Cheetah                  = 107,
    Sapphire                 = 108,
    Infernus                 = 109,
    Fufu                     = 110,
    Peach                    = 111,
    Liz                      = 112,
    Downtown_Cab_Co_E_248    = 113,
    Cinema_Doppler           = 114,
    Ten_Cent_Theater         = 115,
    Tivoli_Cinema            = 116,
    Car_Scrapyard            = 117,
    Smoke_on_the_Water       = 118,
    Tequi_la_la              = 119,
    Pitchers                 = 120,
    The_Hen_House            = 121,
    Hookies                  = 122,
    Los_Santos_Golf_Club     = 123,
    Los_Santos_Customs_E_264 = 124,
    Towing_Impound           = 125,
    McKenzie_Field_Hangar    = 126,
    Sonar_Collections_Dock   = 127,
    Detonate                 = 128,
    Jay_Norris               = 129,
    Emergency_Services       = 130,
    Downtown_Cab_Co_163      = 131,
    Chat_Call                = 132,
    Space_Monkey             = 133,
    Isaac                    = 134,
    Castro                   = 135,
    CELL_ARTU                = 136,
    Unknown                  = 137,
    Legendary_Motorsport     = 138,
    DockTease                = 139,
    Warstock                 = 140,
    Elitas_Travel            = 141,
    P_M_Cycles               = 142,
    Southern_SA_Super_Autos  = 143,
    Virtual_MP               = 144,
    _                        = 145,
    CELL_E_284               = 146,
    Dummy_1                  = 147,
};

enum class eHeistId
    {
        HEIST_JEWELRY,
        HEIST_DOCKS,
        HEIST_RURAL,
        HEIST_AGENCY,
        HEIST_FINALE
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
        } BITS_TriggerFlags;
    };
    uint32_t field_0x5c;

    union
    {
        uint32_t nFriendsBitset;

        struct
        {
            bool Michael : 1;
            bool Franklin : 1;
            bool Trevor : 1;
            bool Lamar : 1;
            bool Amanda : 1;
            bool Jimmy : 1;
        } BITS_FriendsBitset;
    };
    uint32_t field_0x64[5];

    union
    {
        uint32_t nMissionFlags;

        struct
        {
            bool FLAG_0 : 1;
            bool FLAG_1 : 1;
            bool NO_MISSION_RETRY : 1;
            bool FLAG_3 : 1;
            bool NO_STAT_WATCHER : 1;
            bool FLAG_5 : 1;
            bool FLAG_6 : 1;
            bool FLAG_7 : 1;
            bool FLAG_8 : 1;
            bool FLAG_9 : 1;
            bool FLAG_10 : 1;
            bool FLAG_11 : 1;
            bool FLAG_12 : 1;
            bool FLAG_13 : 1;
            bool IS_HEIST_MISSION : 1;
            bool FLAG_15 : 1;
            bool DISABLE_MISSION_SKIPS : 1;
            bool DISABLE_MISSION_EXIT : 1;
            
        } BITS_MissionFlags;
    };
    uint8_t field_0x7c[148];
};
static_assert (sizeof (MissionDefinition) == 0x22 * 8);

/*******************************************************/
struct MissionFlowCommand
{
    alignas(8) uint32_t CommandHash;
    alignas(8) uint32_t Type;
    alignas(8) uint32_t Data;
};

/*******************************************************/
template <typename T> struct YscArray
{
    uint64_t Size = 0;
    T        Data[1];

    T &
    operator[] (uint64_t idx)
    {
        return Data[idx];
    }
};

/*******************************************************/
/*******************************************************/
class MissionRandomizer_GlobalsManager
{
public:
    // PP_INFO.AVAILABLE
    inline static bool *SP0_AVAILABLE = nullptr;
    inline static bool *SP1_AVAILABLE = nullptr;
    inline static bool *SP2_AVAILABLE = nullptr;

    // PP_INFO.PP_CURRENT_PED
    inline static int *PP_CURRENT_PED       = nullptr;
    inline static int *Crew_Unlocked_Bitset = nullptr;
    inline static int *Crew_Dead_Bitset     = nullptr;

    inline static void *MF_MISSION_STRUCT_99 = nullptr;

    // MF_CONTROLS_STRUCT
    inline static YscArray<uint64_t> *MF_CONTROL_FLAGIDS = nullptr;
    inline static YscArray<uint64_t> *MF_CONTROL_INTIDS  = nullptr;

    YscUtils::ScriptGlobal<YscArray<MissionDefinition>> g_Missions{
        "2d 09 0b 00 ? 38 01 38 00 5e ? ? ? 34 22", 10,
        "standard_global_init"_joaat};

    YscUtils::ScriptGlobal<uint32_t> g_CurrentMission{
        "2c 04 ? ? 38 02 60 ? ? ? 2e 05 00", 7, "flow_controller"_joaat, -1u};

    YscUtils::ScriptGlobal<uint32_t> g_LastPassedMission{
        "38 ? 60 ? ? ? 2c ? ? ? 60 ? ? ? 38 ? 25 1c", 3,
        "flow_controller"_joaat, -1u};

    YscUtils::ScriptGlobal<uint32_t> g_LastPassedMissionTime{
        "38 ? 60 ? ? ? 2c ? ? ? 60 ? ? ? 38 ? 25 1c", 11,
        "flow_controller"_joaat, -1u};

    YscUtils::ScriptGlobal<YscArray<MissionFlowCommand>> g_MissionFlowCommands{
        "5e ? ? ? 46 ? ? 35 ? 28 6d 05 ad 1f", 1, "flow_controller"_joaat,
        YscUtils::GLOBAL_U24_IOFFSET_S16};

    YscUtils::ScriptGlobal<uint32_t> g_BoardInitStateBitset{
        "2d 01 03 00 ? 5f ? ? ? 38 ? 2c ? ? ? 2e 01 01", 6,
        "jewelry_heist"_joaat};

    // I dunno what other function this global serves other than making
    // mission_triggerer force you into walking, so yeah :P
    YscUtils::ScriptGlobal<uint32_t> g_ForceWalking{
        "5f ? ? ? 56 ? ? 5f ? ? ? 38 05 57 ? ? 38 05 ", 1,
        "flow_controller"_joaat, 0};
    
    /*******************************************************/
    template <typename T, int fieldNameIdx = 1>
    static void
    StoreSaveGlobals (scrThread::Info *info)
    {
        using namespace std::string_literals;

        const char *fieldName = info->GetArg<const char *> (fieldNameIdx);
        T *         ptr       = info->GetArg<T *> (0);

#define ADD_SAVE_DATA_GLOBAL(global)                                           \
    if (fieldName == #global##s)                                               \
        global = static_cast<decltype(global)>(ptr)

        if constexpr (std::is_same_v<T, bool>)
            {
                ADD_SAVE_DATA_GLOBAL (SP0_AVAILABLE);
                ADD_SAVE_DATA_GLOBAL (SP1_AVAILABLE);
                ADD_SAVE_DATA_GLOBAL (SP2_AVAILABLE);
            }
        else if constexpr (std::is_same_v<T, int>)
            {
                ADD_SAVE_DATA_GLOBAL (PP_CURRENT_PED);
                ADD_SAVE_DATA_GLOBAL (Crew_Unlocked_Bitset);
                ADD_SAVE_DATA_GLOBAL (Crew_Dead_Bitset);
            }
        else if constexpr (std::is_same_v<T, void>)
            {
                ADD_SAVE_DATA_GLOBAL (MF_MISSION_STRUCT_99);
                ADD_SAVE_DATA_GLOBAL (MF_CONTROL_FLAGIDS);
                ADD_SAVE_DATA_GLOBAL (MF_CONTROL_INTIDS);
            }

#undef ADD_SAVE_DATA_GLOBAL
    }

    /*******************************************************/
    static ePlayerIndex
    GetCurrentPlayer ()
    {
        if (*PP_CURRENT_PED < 4)
            return ePlayerIndex (*PP_CURRENT_PED);

        return ePlayerIndex::PLAYER_UNKNOWN;
    }

    /*******************************************************/
    uint64_t &
    GetMfInt (eMissionFlowControlIntId id)
    {
        return MF_CONTROL_INTIDS->Data[id];
    }

    /*******************************************************/
    uint64_t &
    GetMfFlag (eFlowFlag id)
    {
        return MF_CONTROL_FLAGIDS->Data[id];
    }

    /*******************************************************/
    bool
    Process (scrProgram *program, scrThreadContext *ctx)
    {
        g_CurrentMission.Init (program);
        g_LastPassedMission.Init (program);
        g_LastPassedMissionTime.Init (program);
        g_ForceWalking.Init (program);
        g_MissionFlowCommands.Init (program);
        g_Missions.Init (program);

        return true;
    }

    /*******************************************************/
    void
    Initialise ()
    {
#define HOOK(native, func)                                                     \
    NativeCallbackMgr::InitCallback<native##_joaat, func, true> ()
#define s_StoreSaveGlobals(...) StoreSaveGlobals<__VA_ARGS__>

        HOOK ("REGISTER_BOOL_TO_SAVE", s_StoreSaveGlobals (bool));
        HOOK ("REGISTER_ENUM_TO_SAVE", s_StoreSaveGlobals (int));
        HOOK ("REGISTER_INT_TO_SAVE", s_StoreSaveGlobals (int));
        HOOK ("_START_SAVE_STRUCT", s_StoreSaveGlobals (void, 2));
        HOOK ("_START_SAVE_ARRAY", s_StoreSaveGlobals (void, 2));

#undef HOOK
#undef s_StoreSaveGlobals
    }
};
