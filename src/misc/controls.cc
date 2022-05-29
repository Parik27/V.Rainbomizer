#include "CTheScripts.hh"
#include "Patterns/Patterns.hh"
#include "common/logger.hh"
#include "injector/hooking.hpp"
#include "rage.hh"
#include <Utils.hh>
#include <ParserUtils.hh>
#include <map>
#include <stdint.h>
#include <common/minhook.hh>
#include <vadefs.h>

void (*ReloadControls)();

struct ioValue
{
    float    fMultiplier;
    float    fValue;
    uint32_t field2_0x8;
    float    fNextValue;
    uint32_t nSource;
    int      nParameter;
    uint32_t nDeviceId;
    int      field7_0x1c;
    int      field8_0x20;
    uint32_t field9_0x24;
    uint32_t nNextSource;
    uint32_t nNextParameter;
    uint32_t nNextDeviceId;
    uint32_t field13_0x34;
    uint16_t field14_0x38;
    uint8_t  Flags;
    uint32_t field16_0x3c;
    void    *pHistory;
};

typedef enum eControl {
    INPUT_NEXT_CAMERA=0,
    INPUT_LOOK_LR=1,
    INPUT_LOOK_UD=2,
    INPUT_LOOK_UP_ONLY=3,
    INPUT_LOOK_DOWN_ONLY=4,
    INPUT_LOOK_LEFT_ONLY=5,
    INPUT_LOOK_RIGHT_ONLY=6,
    INPUT_CINEMATIC_SLOWMO=7,
    INPUT_SCRIPTED_FLY_UD=8,
    INPUT_SCRIPTED_FLY_LR=9,
    INPUT_SCRIPTED_FLY_ZUP=10,
    INPUT_SCRIPTED_FLY_ZDOWN=11,
    INPUT_WEAPON_WHEEL_UD=12,
    INPUT_WEAPON_WHEEL_LR=13,
    INPUT_WEAPON_WHEEL_NEXT=14,
    INPUT_WEAPON_WHEEL_PREV=15,
    INPUT_SELECT_NEXT_WEAPON=16,
    INPUT_SELECT_PREV_WEAPON=17,
    INPUT_SKIP_CUTSCENE=18,
    INPUT_CHARACTER_WHEEL=19,
    INPUT_MULTIPLAYER_INFO=20,
    INPUT_SPRINT=21,
    INPUT_JUMP=22,
    INPUT_ENTER=23,
    INPUT_ATTACK=24,
    INPUT_AIM=25,
    INPUT_LOOK_BEHIND=26,
    INPUT_PHONE=27,
    INPUT_SPECIAL_ABILITY=28,
    INPUT_SPECIAL_ABILITY_SECONDARY=29,
    INPUT_MOVE_LR=30,
    INPUT_MOVE_UD=31,
    INPUT_MOVE_UP_ONLY=32,
    INPUT_MOVE_DOWN_ONLY=33,
    INPUT_MOVE_LEFT_ONLY=34,
    INPUT_MOVE_RIGHT_ONLY=35,
    INPUT_DUCK=36,
    INPUT_SELECT_WEAPON=37,
    INPUT_PICKUP=38,
    INPUT_SNIPER_ZOOM=39,
    INPUT_SNIPER_ZOOM_IN_ONLY=40,
    INPUT_SNIPER_ZOOM_OUT_ONLY=41,
    INPUT_SNIPER_ZOOM_IN_SECONDARY=42,
    INPUT_SNIPER_ZOOM_OUT_SECONDARY=43,
    INPUT_COVER=44,
    INPUT_RELOAD=45,
    INPUT_TALK=46,
    INPUT_DETONATE=47,
    INPUT_HUD_SPECIAL=48,
    INPUT_ARREST=49,
    INPUT_ACCURATE_AIM=50,
    INPUT_CONTEXT=51,
    INPUT_CONTEXT_SECONDARY=52,
    INPUT_WEAPON_SPECIAL=53,
    INPUT_WEAPON_SPECIAL_TWO=54,
    INPUT_DIVE=55,
    INPUT_DROP_WEAPON=56,
    INPUT_DROP_AMMO=57,
    INPUT_THROW_GRENADE=58,
    INPUT_VEH_MOVE_LR=59,
    INPUT_VEH_MOVE_UD=60,
    INPUT_VEH_MOVE_UP_ONLY=61,
    INPUT_VEH_MOVE_DOWN_ONLY=62,
    INPUT_VEH_MOVE_LEFT_ONLY=63,
    INPUT_VEH_MOVE_RIGHT_ONLY=64,
    INPUT_VEH_SPECIAL=65,
    INPUT_VEH_GUN_LR=66,
    INPUT_VEH_GUN_UD=67,
    INPUT_VEH_AIM=68,
    INPUT_VEH_ATTACK=69,
    INPUT_VEH_ATTACK2=70,
    INPUT_VEH_ACCELERATE=71,
    INPUT_VEH_BRAKE=72,
    INPUT_VEH_DUCK=73,
    INPUT_VEH_HEADLIGHT=74,
    INPUT_VEH_EXIT=75,
    INPUT_VEH_HANDBRAKE=76,
    INPUT_VEH_HOTWIRE_LEFT=77,
    INPUT_VEH_HOTWIRE_RIGHT=78,
    INPUT_VEH_LOOK_BEHIND=79,
    INPUT_VEH_CIN_CAM=80,
    INPUT_VEH_NEXT_RADIO=81,
    INPUT_VEH_PREV_RADIO=82,
    INPUT_VEH_NEXT_RADIO_TRACK=83,
    INPUT_VEH_PREV_RADIO_TRACK=84,
    INPUT_VEH_RADIO_WHEEL=85,
    INPUT_VEH_HORN=86,
    INPUT_VEH_FLY_THROTTLE_UP=87,
    INPUT_VEH_FLY_THROTTLE_DOWN=88,
    INPUT_VEH_FLY_YAW_LEFT=89,
    INPUT_VEH_FLY_YAW_RIGHT=90,
    INPUT_VEH_PASSENGER_AIM=91,
    INPUT_VEH_PASSENGER_ATTACK=92,
    INPUT_VEH_SPECIAL_ABILITY_FRANKLIN=93,
    INPUT_VEH_STUNT_UD=94,
    INPUT_VEH_CINEMATIC_UD=95,
    INPUT_VEH_CINEMATIC_UP_ONLY=96,
    INPUT_VEH_CINEMATIC_DOWN_ONLY=97,
    INPUT_VEH_CINEMATIC_LR=98,
    INPUT_VEH_SELECT_NEXT_WEAPON=99,
    INPUT_VEH_SELECT_PREV_WEAPON=100,
    INPUT_VEH_ROOF=101,
    INPUT_VEH_JUMP=102,
    INPUT_VEH_GRAPPLING_HOOK=103,
    INPUT_VEH_SHUFFLE=104,
    INPUT_VEH_DROP_PROJECTILE=105,
    INPUT_VEH_MOUSE_CONTROL_OVERRIDE=106,
    INPUT_VEH_FLY_ROLL_LR=107,
    INPUT_VEH_FLY_ROLL_LEFT_ONLY=108,
    INPUT_VEH_FLY_ROLL_RIGHT_ONLY=109,
    INPUT_VEH_FLY_PITCH_UD=110,
    INPUT_VEH_FLY_PITCH_UP_ONLY=111,
    INPUT_VEH_FLY_PITCH_DOWN_ONLY=112,
    INPUT_VEH_FLY_UNDERCARRIAGE=113,
    INPUT_VEH_FLY_ATTACK=114,
    INPUT_VEH_FLY_SELECT_NEXT_WEAPON=115,
    INPUT_VEH_FLY_SELECT_PREV_WEAPON=116,
    INPUT_VEH_FLY_SELECT_TARGET_LEFT=117,
    INPUT_VEH_FLY_SELECT_TARGET_RIGHT=118,
    INPUT_VEH_FLY_VERTICAL_FLIGHT_MODE=119,
    INPUT_VEH_FLY_DUCK=120,
    INPUT_VEH_FLY_ATTACK_CAMERA=121,
    INPUT_VEH_FLY_MOUSE_CONTROL_OVERRIDE=122,
    INPUT_VEH_SUB_TURN_LR=123,
    INPUT_VEH_SUB_TURN_LEFT_ONLY=124,
    INPUT_VEH_SUB_TURN_RIGHT_ONLY=125,
    INPUT_VEH_SUB_PITCH_UD=126,
    INPUT_VEH_SUB_PITCH_UP_ONLY=127,
    INPUT_VEH_SUB_PITCH_DOWN_ONLY=128,
    INPUT_VEH_SUB_THROTTLE_UP=129,
    INPUT_VEH_SUB_THROTTLE_DOWN=130,
    INPUT_VEH_SUB_ASCEND=131,
    INPUT_VEH_SUB_DESCEND=132,
    INPUT_VEH_SUB_TURN_HARD_LEFT=133,
    INPUT_VEH_SUB_TURN_HARD_RIGHT=134,
    INPUT_VEH_SUB_MOUSE_CONTROL_OVERRIDE=135,
    INPUT_VEH_PUSHBIKE_PEDAL=136,
    INPUT_VEH_PUSHBIKE_SPRINT=137,
    INPUT_VEH_PUSHBIKE_FRONT_BRAKE=138,
    INPUT_VEH_PUSHBIKE_REAR_BRAKE=139,
    INPUT_MELEE_ATTACK_LIGHT=140,
    INPUT_MELEE_ATTACK_HEAVY=141,
    INPUT_MELEE_ATTACK_ALTERNATE=142,
    INPUT_MELEE_BLOCK=143,
    INPUT_PARACHUTE_DEPLOY=144,
    INPUT_PARACHUTE_DETACH=145,
    INPUT_PARACHUTE_TURN_LR=146,
    INPUT_PARACHUTE_TURN_LEFT_ONLY=147,
    INPUT_PARACHUTE_TURN_RIGHT_ONLY=148,
    INPUT_PARACHUTE_PITCH_UD=149,
    INPUT_PARACHUTE_PITCH_UP_ONLY=150,
    INPUT_PARACHUTE_PITCH_DOWN_ONLY=151,
    INPUT_PARACHUTE_BRAKE_LEFT=152,
    INPUT_PARACHUTE_BRAKE_RIGHT=153,
    INPUT_PARACHUTE_SMOKE=154,
    INPUT_PARACHUTE_PRECISION_LANDING=155,
    INPUT_MAP=156,
    INPUT_SELECT_WEAPON_UNARMED=157,
    INPUT_SELECT_WEAPON_MELEE=158,
    INPUT_SELECT_WEAPON_HANDGUN=159,
    INPUT_SELECT_WEAPON_SHOTGUN=160,
    INPUT_SELECT_WEAPON_SMG=161,
    INPUT_SELECT_WEAPON_AUTO_RIFLE=162,
    INPUT_SELECT_WEAPON_SNIPER=163,
    INPUT_SELECT_WEAPON_HEAVY=164,
    INPUT_SELECT_WEAPON_SPECIAL=165,
    INPUT_SELECT_CHARACTER_MICHAEL=166,
    INPUT_SELECT_CHARACTER_FRANKLIN=167,
    INPUT_SELECT_CHARACTER_TREVOR=168,
    INPUT_SELECT_CHARACTER_MULTIPLAYER=169,
    INPUT_SAVE_REPLAY_CLIP=170,
    INPUT_SPECIAL_ABILITY_PC=171,
    INPUT_CELLPHONE_UP=172,
    INPUT_CELLPHONE_DOWN=173,
    INPUT_CELLPHONE_LEFT=174,
    INPUT_CELLPHONE_RIGHT=175,
    INPUT_CELLPHONE_SELECT=176,
    INPUT_CELLPHONE_CANCEL=177,
    INPUT_CELLPHONE_OPTION=178,
    INPUT_CELLPHONE_EXTRA_OPTION=179,
    INPUT_CELLPHONE_SCROLL_FORWARD=180,
    INPUT_CELLPHONE_SCROLL_BACKWARD=181,
    INPUT_CELLPHONE_CAMERA_FOCUS_LOCK=182,
    INPUT_CELLPHONE_CAMERA_GRID=183,
    INPUT_CELLPHONE_CAMERA_SELFIE=184,
    INPUT_CELLPHONE_CAMERA_DOF=185,
    INPUT_CELLPHONE_CAMERA_EXPRESSION=186,
    INPUT_FRONTEND_DOWN=187,
    INPUT_FRONTEND_UP=188,
    INPUT_FRONTEND_LEFT=189,
    INPUT_FRONTEND_RIGHT=190,
    INPUT_FRONTEND_RDOWN=191,
    INPUT_FRONTEND_RUP=192,
    INPUT_FRONTEND_RLEFT=193,
    INPUT_FRONTEND_RRIGHT=194,
    INPUT_FRONTEND_AXIS_X=195,
    INPUT_FRONTEND_AXIS_Y=196,
    INPUT_FRONTEND_RIGHT_AXIS_X=197,
    INPUT_FRONTEND_RIGHT_AXIS_Y=198,
    INPUT_FRONTEND_PAUSE=199,
    INPUT_FRONTEND_PAUSE_ALTERNATE=200,
    INPUT_FRONTEND_ACCEPT=201,
    INPUT_FRONTEND_CANCEL=202,
    INPUT_FRONTEND_X=203,
    INPUT_FRONTEND_Y=204,
    INPUT_FRONTEND_LB=205,
    INPUT_FRONTEND_RB=206,
    INPUT_FRONTEND_LT=207,
    INPUT_FRONTEND_RT=208,
    INPUT_FRONTEND_LS=209,
    INPUT_FRONTEND_RS=210,
    INPUT_FRONTEND_LEADERBOARD=211,
    INPUT_FRONTEND_SOCIAL_CLUB=212,
    INPUT_FRONTEND_SOCIAL_CLUB_SECONDARY=213,
    INPUT_FRONTEND_DELETE=214,
    INPUT_FRONTEND_ENDSCREEN_ACCEPT=215,
    INPUT_FRONTEND_ENDSCREEN_EXPAND=216,
    INPUT_FRONTEND_SELECT=217,
    INPUT_SCRIPT_LEFT_AXIS_X=218,
    INPUT_SCRIPT_LEFT_AXIS_Y=219,
    INPUT_SCRIPT_RIGHT_AXIS_X=220,
    INPUT_SCRIPT_RIGHT_AXIS_Y=221,
    INPUT_SCRIPT_RUP=222,
    INPUT_SCRIPT_RDOWN=223,
    INPUT_SCRIPT_RLEFT=224,
    INPUT_SCRIPT_RRIGHT=225,
    INPUT_SCRIPT_LB=226,
    INPUT_SCRIPT_RB=227,
    INPUT_SCRIPT_LT=228,
    INPUT_SCRIPT_RT=229,
    INPUT_SCRIPT_LS=230,
    INPUT_SCRIPT_RS=231,
    INPUT_SCRIPT_PAD_UP=232,
    INPUT_SCRIPT_PAD_DOWN=233,
    INPUT_SCRIPT_PAD_LEFT=234,
    INPUT_SCRIPT_PAD_RIGHT=235,
    INPUT_SCRIPT_SELECT=236,
    INPUT_CURSOR_ACCEPT=237,
    INPUT_CURSOR_CANCEL=238,
    INPUT_CURSOR_X=239,
    INPUT_CURSOR_Y=240,
    INPUT_CURSOR_SCROLL_UP=241,
    INPUT_CURSOR_SCROLL_DOWN=242,
    INPUT_ENTER_CHEAT_CODE=243,
    INPUT_INTERACTION_MENU=244,
    INPUT_MP_TEXT_CHAT_ALL=245,
    INPUT_MP_TEXT_CHAT_TEAM=246,
    INPUT_MP_TEXT_CHAT_FRIENDS=247,
    INPUT_MP_TEXT_CHAT_CREW=248,
    INPUT_PUSH_TO_TALK=249,
    INPUT_CREATOR_LS=250,
    INPUT_CREATOR_RS=251,
    INPUT_CREATOR_LT=252,
    INPUT_CREATOR_RT=253,
    INPUT_CREATOR_MENU_TOGGLE=254,
    INPUT_CREATOR_ACCEPT=255,
    INPUT_CREATOR_DELETE=256,
    INPUT_ATTACK2=257,
    INPUT_RAPPEL_JUMP=258,
    INPUT_RAPPEL_LONG_JUMP=259,
    INPUT_RAPPEL_SMASH_WINDOW=260,
    INPUT_PREV_WEAPON=261,
    INPUT_NEXT_WEAPON=262,
    INPUT_MELEE_ATTACK1=263,
    INPUT_MELEE_ATTACK2=264,
    INPUT_WHISTLE=265,
    INPUT_MOVE_LEFT=266,
    INPUT_MOVE_RIGHT=267,
    INPUT_MOVE_UP=268,
    INPUT_MOVE_DOWN=269,
    INPUT_LOOK_LEFT=270,
    INPUT_LOOK_RIGHT=271,
    INPUT_LOOK_UP=272,
    INPUT_LOOK_DOWN=273,
    INPUT_SNIPER_ZOOM_IN=274,
    INPUT_SNIPER_ZOOM_OUT=275,
    INPUT_SNIPER_ZOOM_IN_ALTERNATE=276,
    INPUT_SNIPER_ZOOM_OUT_ALTERNATE=277,
    INPUT_VEH_MOVE_LEFT=278,
    INPUT_VEH_MOVE_RIGHT=279,
    INPUT_VEH_MOVE_UP=280,
    INPUT_VEH_MOVE_DOWN=281,
    INPUT_VEH_GUN_LEFT=282,
    INPUT_VEH_GUN_RIGHT=283,
    INPUT_VEH_GUN_UP=284,
    INPUT_VEH_GUN_DOWN=285,
    INPUT_VEH_LOOK_LEFT=286,
    INPUT_VEH_LOOK_RIGHT=287,
    INPUT_REPLAY_START_STOP_RECORDING=288,
    INPUT_REPLAY_START_STOP_RECORDING_SECONDARY=289,
    INPUT_SCALED_LOOK_LR=290,
    INPUT_SCALED_LOOK_UD=291,
    INPUT_SCALED_LOOK_UP_ONLY=292,
    INPUT_SCALED_LOOK_DOWN_ONLY=293,
    INPUT_SCALED_LOOK_LEFT_ONLY=294,
    INPUT_SCALED_LOOK_RIGHT_ONLY=295,
    INPUT_REPLAY_MARKER_DELETE=296,
    INPUT_REPLAY_CLIP_DELETE=297,
    INPUT_REPLAY_PAUSE=298,
    INPUT_REPLAY_REWIND=299,
    INPUT_REPLAY_FFWD=300,
    INPUT_REPLAY_NEWMARKER=301,
    INPUT_REPLAY_RECORD=302,
    INPUT_REPLAY_SCREENSHOT=303,
    INPUT_REPLAY_HIDEHUD=304,
    INPUT_REPLAY_STARTPOINT=305,
    INPUT_REPLAY_ENDPOINT=306,
    INPUT_REPLAY_ADVANCE=307,
    INPUT_REPLAY_BACK=308,
    INPUT_REPLAY_TOOLS=309,
    INPUT_REPLAY_RESTART=310,
    INPUT_REPLAY_SHOWHOTKEY=311,
    INPUT_REPLAY_CYCLEMARKERLEFT=312,
    INPUT_REPLAY_CYCLEMARKERRIGHT=313,
    INPUT_REPLAY_FOVINCREASE=314,
    INPUT_REPLAY_FOVDECREASE=315,
    INPUT_REPLAY_CAMERAUP=316,
    INPUT_REPLAY_CAMERADOWN=317,
    INPUT_REPLAY_SAVE=318,
    INPUT_REPLAY_TOGGLETIME=319,
    INPUT_REPLAY_TOGGLETIPS=320,
    INPUT_REPLAY_PREVIEW=321,
    INPUT_REPLAY_TOGGLE_TIMELINE=322,
    INPUT_REPLAY_TIMELINE_PICKUP_CLIP=323,
    INPUT_REPLAY_TIMELINE_DUPLICATE_CLIP=324,
    INPUT_REPLAY_TIMELINE_PLACE_CLIP=325,
    INPUT_REPLAY_CTRL=326,
    INPUT_REPLAY_TIMELINE_SAVE=327,
    INPUT_REPLAY_PREVIEW_AUDIO=328,
    INPUT_VEH_DRIVE_LOOK=329,
    INPUT_VEH_DRIVE_LOOK2=330,
    INPUT_VEH_FLY_ATTACK2=331,
    INPUT_RADIO_WHEEL_UD=332,
    INPUT_RADIO_WHEEL_LR=333,
    INPUT_VEH_SLOWMO_UD=334,
    INPUT_VEH_SLOWMO_UP_ONLY=335,
    INPUT_VEH_SLOWMO_DOWN_ONLY=336,
    INPUT_VEH_HYDRAULICS_CONTROL_TOGGLE=337,
    INPUT_VEH_HYDRAULICS_CONTROL_LEFT=338,
    INPUT_VEH_HYDRAULICS_CONTROL_RIGHT=339,
    INPUT_VEH_HYDRAULICS_CONTROL_UP=340,
    INPUT_VEH_HYDRAULICS_CONTROL_DOWN=341,
    INPUT_VEH_HYDRAULICS_CONTROL_UD=342,
    INPUT_VEH_HYDRAULICS_CONTROL_LR=343,
    INPUT_SWITCH_VISOR=344,
    INPUT_VEH_MELEE_HOLD=345,
    INPUT_VEH_MELEE_LEFT=346,
    INPUT_VEH_MELEE_RIGHT=347,
    INPUT_MAP_POI=348,
    INPUT_REPLAY_SNAPMATIC_PHOTO=349,
    INPUT_VEH_CAR_JUMP=350,
    INPUT_VEH_ROCKET_BOOST=351,
    INPUT_VEH_FLY_BOOST=352,
    INPUT_VEH_PARACHUTE=353,
    INPUT_VEH_BIKE_WINGS=354,
    INPUT_VEH_FLY_BOMB_BAY=355,
    INPUT_VEH_FLY_COUNTER=356,
    INPUT_VEH_TRANSFORM=357,
    INPUT_QUAD_LOCO_REVERSE=358,
    INPUT_RESPAWN_FASTER=359,
    INPUT_HUDMARKER_SELECT=360
} eControl;

class CControls
{
public:
    ioValue *
    GetIoValue (uint32_t idx)
    {
        return ((ioValue *) (&((char *) (this))[33704])) + idx;
    }
};

class rage__ControlInput__Mapping
    : public ParserWrapper<rage__ControlInput__Mapping>
{
};

class rage__ControlInput__ControlSettings
    : public ParserWrapper<rage__ControlInput__ControlSettings>
{
public:
    atArrayGetSizeWrapper<rage__ControlInput__Mapping>&
    GetMappings ()
    {
        return Get<atArrayGetSizeWrapper<rage__ControlInput__Mapping>> (
            "Mappings"_joaat);
    }
};

using ControlSettings = rage__ControlInput__ControlSettings;

class ControlsRandomizer
{
    inline static enum {
        SWAP_NONE,
        SWAP_HORIZONTAL,
        SWAP_ALL
    } sm_InputSwapPreset = SWAP_HORIZONTAL;

    static void
    AdjustControls (ControlSettings *settings)
    {
        std::vector<std::pair<uint32_t, uint32_t>> InputsToSwap;
        if (sm_InputSwapPreset == SWAP_HORIZONTAL)
            {
                InputsToSwap = {
                    // {"INPUT_MOVE_LEFT_ONLY"_joaat,
                    //              "INPUT_MOVE_RIGHT_ONLY"_joaat},
                    //             {"INPUT_SCALED_LOOK_LEFT_ONLY"_joaat,
                    //              "INPUT_SCALED_LOOK_RIGHT_ONLY"_joaat},
                    //             {"INPUT_LOOK_LEFT_ONLY"_joaat,
                    //              "INPUT_LOOK_RIGHT_ONLY"_joaat},
                    //             {"INPUT_VEH_MOVE_LEFT_ONLY"_joaat,
                    //              "INPUT_VEH_MOVE_RIGHT_ONLY"_joaat},
                    {"INPUT_VEH_FLY_YAW_LEFT"_joaat,
                     "INPUT_VEH_FLY_YAW_RIGHT"_joaat},

                    // {"INPUT_VEH_FLY_ROLL_LEFT_ONLY"_joaat,
                    //  "INPUT_VEH_FLY_ROLL_RIGHT_ONLY"_joaat},
                    // {"INPUT_VEH_SUB_TURN_LEFT_ONLY"_joaat,
                    //  "INPUT_VEH_SUB_TURN_RIGHT_ONLY"_joaat},
                    {"INPUT_VEH_SUB_TURN_HARD_LEFT"_joaat,
                     "INPUT_VEH_SUB_TURN_HARD_RIGHT"_joaat},

                    // {"INPUT_PARACHUTE_TURN_LEFT_ONLY"_joaat,
                    //  "INPUT_PARACHUTE_TURN_RIGHT_ONLY"_joaat
                };
            }
        else if (sm_InputSwapPreset == SWAP_ALL)
            {
                InputsToSwap = {
                    {"INPUT_SCALED_LOOK_LEFT_ONLY"_joaat,
                     "INPUT_SCALED_LOOK_RIGHT_ONLY"_joaat},
                    {"INPUT_LOOK_LEFT_ONLY"_joaat,
                     "INPUT_LOOK_RIGHT_ONLY"_joaat},
                    {"INPUT_SCALED_LOOK_UP_ONLY"_joaat,
                     "INPUT_SCALED_LOOK_DOWN_ONLY"_joaat},
                    {"INPUT_LOOK_UP_ONLY"_joaat, "INPUT_LOOK_DOWN_ONLY"_joaat},
                    {"INPUT_WEAPON_WHEEL_NEXT"_joaat,
                     "INPUT_WEAPON_WHEEL_PREV"_joaat},
                    {"INPUT_SELECT_NEXT_WEAPON"_joaat,
                     "INPUT_SELECT_PREV_WEAPON"_joaat},
                    {"INPUT_SPRINT"_joaat, "INPUT_JUMP"_joaat},
                    {"INPUT_ATTACK"_joaat, "INPUT_AIM"_joaat},
                    {"INPUT_MOVE_UP_ONLY"_joaat, "INPUT_MOVE_DOWN_ONLY"_joaat},
                    {"INPUT_MOVE_LEFT_ONLY"_joaat,
                     "INPUT_MOVE_RIGHT_ONLY"_joaat},
                    {"INPUT_SNIPER_ZOOM_IN_ONLY"_joaat,
                     "INPUT_SNIPER_ZOOM_OUT_ONLY"_joaat},
                    {"INPUT_VEH_AIM"_joaat, "INPUT_VEH_ATTACK"_joaat},
                    {"INPUT_VEH_BRAKE"_joaat, "INPUT_VEH_ACCELERATE"_joaat},
                    {"INPUT_VEH_MOVE_LEFT_ONLY"_joaat,
                     "INPUT_VEH_MOVE_RIGHT_ONLY"_joaat},
                    {"INPUT_CELLPHONE_SELECT"_joaat,
                     "INPUT_CELLPHONE_CANCEL"_joaat}};
            }
        
        for (auto &mapping : settings->GetMappings ())
            {
                auto input = mapping.Equate ("Input"_joaat);
                for (auto swaps : InputsToSwap)
                    {
                        if (swaps.first == input.ToHash ())
                            input    = swaps.second;
                        else if (swaps.second == input.ToHash ())
                            input    = swaps.first;
                    }
            }
    }

    template <auto &O>
    static void
    AdjustControls (CControls *p1, ControlSettings *p2)
    {
        AdjustControls (p2);
        O (p1, p2);
    }

    template <auto &O>
    static void
    AdjustControls2 (CControls *p1, ControlSettings *p2, int p3)
    {
        AdjustControls (p2);
        O (p1, p2, p3);
    }

    template <auto &O>
    static void
    AdjustControls3 (CControls *p1, const char *name, bool p3,
                     ControlSettings *p4)
    {
        O (p1, name, p3, p4);
        std::string sName = name;
        AdjustControls (p4);

        Rainbomizer::Logger::LogMessage("%s", name);
    }

    template <auto &O>
    static void
    AdjustControls4 (CControls *p1)
    {
        O (p1);
        p1->GetIoValue (INPUT_LOOK_LR)->fMultiplier           = -1;
        p1->GetIoValue (INPUT_MOVE_LR)->fMultiplier           = -1;
        p1->GetIoValue (INPUT_SCALED_LOOK_LR)->fMultiplier    = -1;
        p1->GetIoValue (INPUT_VEH_MOVE_LR)->fMultiplier       = -1;
        p1->GetIoValue (INPUT_VEH_FLY_ROLL_LR)->fMultiplier   = -1;
        p1->GetIoValue (INPUT_VEH_SUB_TURN_LR)->fMultiplier   = -1;
        p1->GetIoValue (INPUT_PARACHUTE_TURN_LR)->fMultiplier = -1;
    }

    static void
    SetControlSwapState (scrThread::Info *info)
    {
        sm_InputSwapPreset
            = static_cast<decltype (sm_InputSwapPreset)> (info->GetArg (0));
        ReloadControls ();
        sm_InputSwapPreset = SWAP_NONE;
    }

public:
    ControlsRandomizer ()
    {
        // Reload function
        ReadCall (hook::get_pattern (
                      "39 05 ? ? ? ? 74 ? 89 05 ? ? ? ? e8 ? ? ? ?", 14),
                  ReloadControls);

        // REGISTER_HOOK ("e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8b c7 ? 8b ce e8 ? ? ? ? ? "
        //                "8d ? ? ? ? ? ? 8b ce e8 ? ? ? ?",
        //                0, AdjustControls2, void, CControls *, ControlSettings *,
        //                int);

        // REGISTER_HOOK ("e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8b c7 ? 8b ce e8 ? ? ? ? ? "
        //                "8d ? ? ? ? ? ? 8b ce e8 ? ? ? ?",
        //                18, AdjustControls2, void, CControls *,
        //                ControlSettings *, int);

        // REGISTER_HOOK ("e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8b c7 ? 8b ce e8 ? ? ? ? ? "
        //                "8d ? ? ? ? ? ? 8b ce e8 ? ? ? ?",
        //                33, AdjustControls, void, CControls *,
        //                ControlSettings *);

        // REGISTER_HOOK ("e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8b c7 ? 8b ce e8 ? ? ? ? ? "
        //                "8d ? ? ? ? ? ? 8b ce e8 ? ? ? ?",
        //                48, AdjustControls, void, CControls *,
        //                ControlSettings*);

        REGISTER_HOOK (
            "72 ? ? 8b ce e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8d ? ? ? ? ? ?", 5,
            AdjustControls4, void, CControls *);

        REGISTER_MH_HOOK_BRANCH (
            "e8 ? ? ? ? bf fc ff ff ff ? 8d ? ? ? ? ? ? 8b ce ", 0,
            AdjustControls3, void, CControls *, const char *, bool,
            ControlSettings *);

        // REGISTER_MH_HOOK_BRANCH (
        //     "e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8b c7 ? 8b ce e8 ? ? ? ? ? "
        //     "8d ? ? ? ? ? ? 8b ce e8 ? ? ? ?",
        //     0, AdjustControls2, void, CControls *, ControlSettings *, int);

        NativeManager::AddNative (0xC00510201122022, SetControlSwapState);
    }
};
//controls;
