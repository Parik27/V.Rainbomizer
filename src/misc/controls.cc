#include "CTheScripts.hh"
#include "Patterns/Patterns.hh"
#include "common/logger.hh"
#include "injector/hooking.hpp"
#include "rage.hh"
#include <Utils.hh>
#include <ParserUtils.hh>
#include <map>
#include <stdint.h>

void (*ReloadControls)();

class CControls;

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
    } sm_InputSwapPreset = SWAP_NONE;

    static void
    AdjustControls (ControlSettings *settings)
    {
        std::vector<std::pair<uint32_t, uint32_t>> InputsToSwap;
        if (sm_InputSwapPreset == SWAP_HORIZONTAL)
            {
                InputsToSwap = {{"INPUT_MOVE_LEFT_ONLY"_joaat,
                                 "INPUT_MOVE_RIGHT_ONLY"_joaat},
                                {"INPUT_SCALED_LOOK_LEFT_ONLY"_joaat,
                                 "INPUT_SCALED_LOOK_RIGHT_ONLY"_joaat},
                                {"INPUT_LOOK_LEFT_ONLY"_joaat,
                                 "INPUT_LOOK_RIGHT_ONLY"_joaat},
                                {"INPUT_VEH_MOVE_LEFT_ONLY"_joaat,
                                 "INPUT_VEH_MOVE_RIGHT_ONLY"_joaat},
                                {"INPUT_VEH_FLY_YAW_LEFT"_joaat,
                                 "INPUT_VEH_FLY_YAW_RIGHT"_joaat},
                                {"INPUT_VEH_FLY_ROLL_LEFT_ONLY"_joaat,
                                 "INPUT_VEH_FLY_ROLL_RIGHT_ONLY"_joaat},
                                {"INPUT_VEH_SUB_TURN_LEFT_ONLY"_joaat,
                                 "INPUT_VEH_SUB_TURN_RIGHT_ONLY"_joaat},
                                {"INPUT_VEH_SUB_TURN_HARD_LEFT"_joaat,
                                 "INPUT_VEH_SUB_TURN_HARD_RIGHT"_joaat},
                                {"INPUT_PARACHUTE_TURN_LEFT_ONLY"_joaat,
                                 "INPUT_PARACHUTE_TURN_RIGHT_ONLY"_joaat},
                                {"INPUT_VEH_HYDRAULICS_CONTROL_LEFT"_joaat,
                                 "INPUT_VEH_HYDRAULICS_CONTROL_RIGHT"_joaat}};
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
                            input = swaps.second;
                        else if (swaps.second == input.ToHash ())
                            input = swaps.first;
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

        REGISTER_HOOK ("e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8b c7 ? 8b ce e8 ? ? ? ? ? "
                       "8d ? ? ? ? ? ? 8b ce e8 ? ? ? ?",
                       0, AdjustControls2, void, CControls *, ControlSettings *,
                       int);

        REGISTER_HOOK ("e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8b c7 ? 8b ce e8 ? ? ? ? ? "
                       "8d ? ? ? ? ? ? 8b ce e8 ? ? ? ?",
                       18, AdjustControls2, void, CControls *,
                       ControlSettings *, int);

        REGISTER_HOOK ("e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8b c7 ? 8b ce e8 ? ? ? ? ? "
                       "8d ? ? ? ? ? ? 8b ce e8 ? ? ? ?",
                       33, AdjustControls, void, CControls *,
                       ControlSettings *);

        REGISTER_HOOK ("e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8b c7 ? 8b ce e8 ? ? ? ? ? "
                       "8d ? ? ? ? ? ? 8b ce e8 ? ? ? ?",
                       48, AdjustControls, void, CControls *,
                       ControlSettings *);

        NativeManager::AddNative (0xC00510201122022, SetControlSwapState);
    }
} controls;
