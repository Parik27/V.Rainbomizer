#include "common/logger.hh"
#include "rage.hh"
#include <Utils.hh>
#include <ParserUtils.hh>
#include <map>
#include <stdint.h>

class CControls;

class rage__ControlInput__Mapping
    : public ParserWrapper<rage__ControlInput__Mapping>
{
};

class rage__ControlInput__ControlSettings
    : public ParserWrapper<rage__ControlInput__ControlSettings>
{
public:
    atArrayGetSizeWrapper<rage__ControlInput__Mapping>
    GetMappings ()
    {
        return Get<atArrayGetSizeWrapper<rage__ControlInput__Mapping>> (
            "Mappings"_joaat);
    }
};

using ControlSettings = rage__ControlInput__ControlSettings;

class ControlsRandomizer
{
    static void
    AdjustControls (ControlSettings *settings)
    {
        // clang-format off
        std::vector<std::pair<uint32_t, uint32_t>> InputsToSwap
            = {
            {"INPUT_MOVE_LEFT_ONLY"_joaat              , "INPUT_MOVE_RIGHT_ONLY"_joaat}           ,
               {"INPUT_SCALED_LOOK_LEFT_ONLY"_joaat       , "INPUT_SCALED_LOOK_RIGHT_ONLY"_joaat}    ,
                {"INPUT_LOOK_LEFT_ONLY"_joaat              , "INPUT_LOOK_RIGHT_ONLY"_joaat}           ,
               {"INPUT_VEH_MOVE_LEFT_ONLY"_joaat          , "INPUT_VEH_MOVE_RIGHT_ONLY"_joaat}       ,
               {"INPUT_VEH_FLY_YAW_LEFT"_joaat            , "INPUT_VEH_FLY_YAW_RIGHT"_joaat}         ,
               {"INPUT_VEH_FLY_ROLL_LEFT_ONLY"_joaat      , "INPUT_VEH_FLY_ROLL_RIGHT_ONLY"_joaat}   ,
               {"INPUT_VEH_SUB_TURN_LEFT_ONLY"_joaat      , "INPUT_VEH_SUB_TURN_RIGHT_ONLY"_joaat}   ,
               {"INPUT_VEH_SUB_TURN_HARD_LEFT"_joaat      , "INPUT_VEH_SUB_TURN_HARD_RIGHT"_joaat}   ,
               {"INPUT_PARACHUTE_TURN_LEFT_ONLY"_joaat    , "INPUT_PARACHUTE_TURN_RIGHT_ONLY"_joaat} ,
               {"INPUT_VEH_HYDRAULICS_CONTROL_LEFT"_joaat , "INPUT_VEH_HYDRAULICS_CONTROL_RIGHT"_joaat}
            };
        // clang-format on
        
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

                int* params = mapping.Get<int*>("Parameters"_joaat);
                // if (*params == 512)
                //     *params = 512;
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

public:
    ControlsRandomizer ()
    {
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
    }
} controls;
