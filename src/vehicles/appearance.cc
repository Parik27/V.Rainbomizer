#include <map>
#include <array>

#include <Utils.hh>

#include <common/config.hh>
#include <common/logger.hh>
#include <common/minhook.hh>

#include <HSL.hh>
#include <CARGB.hh>
#include <CVehicle.hh>
#include <CTheScripts.hh>

uint32_t (*CCustomShaderEffectVehicle_SetForVehicle_134) (
    CCustomShaderEffectVehicle *, CVehicle *);

enum eVehicleModType : int32_t
{
    VMT_SPOILER                   = 0,
    VMT_BUMPER_F                  = 1,
    VMT_BUMPER_R                  = 2,
    VMT_SKIRT                     = 3,
    VMT_EXHAUST                   = 4,
    VMT_CHASSIS                   = 5,
    VMT_GRILL                     = 6,
    VMT_BONNET                    = 7,
    VMT_WING_L                    = 8,
    VMT_WING_R                    = 9,
    VMT_ROOF                      = 10,
    VMT_ENGINE                    = 11,
    VMT_BRAKES                    = 12,
    VMT_GEARBOX                   = 13,
    VMT_HORN                      = 14,
    VMT_SUSPENSION                = 15,
    VMT_ARMOUR                    = 16,
    VMT_NITROUS                   = 17,
    VMT_TURBO                     = 18,
    VMT_SUBWOOFER                 = 19,
    VMT_TYRE_SMOKE                = 20,
    VMT_HYDRAULICS                = 21,
    VMT_XENON_LIGHTS              = 22,
    VMT_WHEELS                    = 23,
    VMT_WHEELS_REAR_OR_HYDRAULICS = 24,
    VMT_PLTHOLDER                 = 25,
    VMT_PLTVANITY                 = 26,
    VMT_INTERIOR1                 = 27,
    VMT_INTERIOR2                 = 28,
    VMT_INTERIOR3                 = 29,
    VMT_INTERIOR4                 = 30,
    VMT_INTERIOR5                 = 31,
    VMT_SEATS                     = 32,
    VMT_STEERING                  = 33,
    VMT_KNOB                      = 34,
    VMT_PLAQUE                    = 35,
    VMT_ICE                       = 36,
    VMT_TRUNK                     = 37,
    VMT_HYDRO                     = 38,
    VMT_ENGINEBAY1                = 39,
    VMT_ENGINEBAY2                = 40,
    VMT_ENGINEBAY3                = 41,
    VMT_CHASSIS2                  = 42,
    VMT_CHASSIS3                  = 43,
    VMT_CHASSIS4                  = 44,
    VMT_CHASSIS5                  = 45,
    VMT_DOOR_L                    = 46,
    VMT_DOOR_R                    = 47,
    VMT_LIVERY_MOD                = 48,
    VMT_LIGHTBAR                  = 49,
    VMT_NUM_VEHICLE_MOD_TYPES     = 50
};

class VehicleAppearanceRandomizer
{
    struct VehicleColourData
    {
        CARGB OriginalColours[4];
        CARGB RandomColours[4];
    };

    inline static std::map<CVehicle *, VehicleColourData> mColourData;

    inline static std::array<int, VMT_NUM_VEHICLE_MOD_TYPES> mRandomizeOdds
        = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    RB_C_CONFIG_START
    {
        bool RandomizeCarColours;
        bool RandomizeCarUpgrades;

        int RandomizeHornOdds   = 45;
        int RandomizeArmourOdds = 45;
    }
    RB_C_CONFIG_END

    /*******************************************************/
    static void
    RestoreVehicleColourData (CCustomShaderEffectVehicle *shader, CVehicle *veh)
    {
        if (auto *data = LookupMap (mColourData, veh))
            {
                CARGB *colours = shader->GetColours ();

                for (int i = 0; i < 4; i++)
                    colours[i] = data->OriginalColours[i];
            }
    }

    /*******************************************************/
    static bool
    StoreVehicleColourData (CCustomShaderEffectVehicle *shader, CVehicle *veh)
    {
        auto  &data    = mColourData[veh];
        CARGB *colours = shader->GetColours ();

        bool changed = false;
        for (int i = 0; i < 4; i++)
            if (std::exchange (data.OriginalColours[i], colours[i])
                != colours[i])
                changed = true;

        return changed;
    }

    /*******************************************************/
    inline static bool
    ShouldRandomizeMod (int modIndex)
    {
        return mRandomizeOdds[modIndex] == -1
               || RandomBool (mRandomizeOdds[modIndex]);
    }

    /*******************************************************/
    static void
    RandomizeVehicleUpgrades (CVehicle *vehPtr)
    {
        using namespace NativeLiterals;

        uint32_t veh = fwScriptGuid::CreateGuid (vehPtr);

        "SET_VEHICLE_MOD_KIT"_n(veh, 0);

        for (int i = VMT_SPOILER; i < VMT_NUM_VEHICLE_MOD_TYPES; i++)
            {
                int max = "GET_NUM_VEHICLE_MODS"_n(veh, i);

                if (!ShouldRandomizeMod (i))
                    continue;

                // Normal vehicle mods
                if (max > 0)
                    {
                        "SET_VEHICLE_MOD"_n(veh, i, RandomInt (max - 1),
                                            RandomInt (1));
                    }

                // Toggle Vehicle Mods
                switch (i)
                    {
                    case VMT_NITROUS:
                    case VMT_TURBO:
                    case VMT_SUBWOOFER:
                    case VMT_TYRE_SMOKE:
                    case VMT_HYDRAULICS:
                    case VMT_XENON_LIGHTS:
                        "TOGGLE_VEHICLE_MOD"_n(veh, i, RandomInt (1));
                    }
            }
    }

    /*******************************************************/
    static void
    RandomizeVehicleColours (CVehicle *veh)
    {
        using Rainbomizer::HSL;

        for (int i = 0; i < 4; i++)
            mColourData[veh].RandomColours[i]
                = HSL (RandomFloat (360.0f), 1.0f, RandomFloat (1.0f));
    }

    /*******************************************************/
    static uint32_t
    RandomizeVehicleAppearance (CCustomShaderEffectVehicle *shader,
                                CVehicle                   *veh)
    {
        RestoreVehicleColourData (shader, veh);

        uint32_t ret
            = CCustomShaderEffectVehicle_SetForVehicle_134 (shader, veh);

        CARGB *colours = shader->GetColours ();
        if (StoreVehicleColourData (shader, veh))
            {
                RandomizeVehicleUpgrades (veh);
                RandomizeVehicleColours (veh);
            }

        for (int i = 0; i < 4; i++)
            colours[i] = mColourData[veh].RandomColours[i];

        return ret;
    }

public:
    /*******************************************************/
    VehicleAppearanceRandomizer ()
    {
        RB_C_DO_CONFIG ("VehicleAppearanceRandomizer", RandomizeCarColours,
                        RandomizeCarUpgrades, RandomizeHornOdds,
                        RandomizeArmourOdds);

        mRandomizeOdds[VMT_ARMOUR] = Config ().RandomizeArmourOdds;
        mRandomizeOdds[VMT_HORN]   = Config ().RandomizeHornOdds;

        MinHookWrapper::HookBranchDestination (
            "85 c9 74 ? ? 8b d3 e8 ? ? ? ? 84 c0 74 ? ? 84 ff 74", 7,
            CCustomShaderEffectVehicle_SetForVehicle_134,
            RandomizeVehicleAppearance);
    }

} g_VehicleAppearanceRandomizer;
