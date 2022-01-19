#pragma once

#include <cstdint>
#include <mutex>
#include <array>
#include <vector>

#include <common/events.hh>
#include <common/parser.hh>
#include <exceptions/exceptions_Vars.hh>

#include <Utils.hh>
#include <CPools.hh>
#include <CVehicle.hh>
#include <CTheScripts.hh>

#include "appearance_Config.hh"

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

class VehicleUpgradesRandomizer : VehicleAppearanceRandomizerConfig
{
    inline static std::vector<uint32_t> mUpgradesQueue;
    inline static std::mutex            mUpgradesQueueMutex;

    inline static char PlatesFileName[] = "LicensePlates.txt";
    using PlatesRandomizer = DataFileSourcedRandomizer<PlatesFileName>;

    inline static PlatesRandomizer sm_PlatesRandomizer;

    inline static std::array<int, VMT_NUM_VEHICLE_MOD_TYPES> mRandomizeOdds
        = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
           -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    /*******************************************************/
    inline static bool
    ShouldRandomizeComponent (int modIndex)
    {
        return mRandomizeOdds[modIndex] == -1
               || RandomBool (mRandomizeOdds[modIndex]);
    }

    /*******************************************************/
    static void
    RandomizeComponentUpgrades (ExceptionVarsRecorder &vars, uint32_t veh,
                                int component)
    {
        using namespace NativeLiterals;

        vars ("Current Component", component);
        int max = "GET_NUM_VEHICLE_MODS"_n(veh, component);

        if (!ShouldRandomizeComponent (component))
            return;

        // Normal vehicle mods
        if (max > 0)
            {
                int mod   = RandomInt (max - 1);
                int wheel = RandomInt (1);

                vars ("Mod", mod);
                vars ("Wheel", wheel);
                "SET_VEHICLE_MOD"_n(veh, component, mod, wheel);
            }

        // Toggle Vehicle Mods
        switch (component)
            {
            case VMT_NITROUS:
            case VMT_TURBO:
            case VMT_SUBWOOFER:
            case VMT_TYRE_SMOKE:
            case VMT_HYDRAULICS:
            case VMT_XENON_LIGHTS:
                "TOGGLE_VEHICLE_MOD"_n(veh, component, RandomInt (1));
            }
    }

    /*******************************************************/
    static void
    RandomizeVehiclePlates (ExceptionVarsRecorder &vars, uint32_t veh)
    {
        using namespace NativeLiterals;

        if (!RandomBool (Config ().RandomizeLicensePlateOdds))
            return;

        static std::string mLastPlate = "";

        "SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX"_n(veh, RandomInt (5));
        vars ("Set number plate index", true);

        if (!sm_PlatesRandomizer.RandomizeObject (mLastPlate))
            return;

        "SET_VEHICLE_NUMBER_PLATE_TEXT"_n(veh, mLastPlate.c_str ());
        vars ("Set number plate text", true);
    }

    /*******************************************************/
    static bool
    RandomizeVehicleUpgrades (uint32_t veh)
    {
        const int MIN_FREE_UPGRADE_SLOTS = 30;
        using namespace NativeLiterals;

        // No room in the pool to load more gfx stuff, defer randomization to
        // next time to prevent crash
        if (CPools::GetVehicleStreamRequestGxtPool ()->GetFree ()
            <= MIN_FREE_UPGRADE_SLOTS)
            return false;

        ExceptionVarsRecorder vars ("Randomize Vehicle Upgrades");
        vars ("Vehicle Hash", int ("GET_ENTITY_MODEL"_n(veh)));

        // In case the vehicle is deleted/replaced with something else
        if (!"IS_ENTITY_A_VEHICLE"_n(veh))
            return true;

        "SET_VEHICLE_MOD_KIT"_n(veh, 0);

        for (int i = VMT_SPOILER; i < VMT_NUM_VEHICLE_MOD_TYPES; i++)
            RandomizeComponentUpgrades (vars, veh, i);

        RandomizeVehiclePlates (vars, veh);

        return true;
    }

    /*******************************************************/
    static void
    ProcessUpgradesQueue (uint64_t *, uint64_t *, scrProgram *,
                          scrThreadContext *ctx)
    {
        if (ctx->m_nScriptHash != "main"_joaat)
            return;

        std::lock_guard guard (mUpgradesQueueMutex);

        if (!mUpgradesQueue.size ())
            return;

        if (RandomizeVehicleUpgrades (mUpgradesQueue.back ()))
            mUpgradesQueue.pop_back ();
    }

public:
    /*******************************************************/
    static void
    EnqueueUpgradesRandomization (CVehicle *vehPtr)
    {
        std::lock_guard guard (mUpgradesQueueMutex);

        uint32_t veh = fwScriptGuid::CreateGuid (vehPtr);
        mUpgradesQueue.push_back (veh);
    }

    /*******************************************************/
    static void
    Init ()
    {
        mRandomizeOdds[VMT_HORN]   = Config ().RandomizeHornOdds;
        mRandomizeOdds[VMT_ARMOUR] = Config ().RandomizeArmourOdds;

        Rainbomizer::Events ().OnRunThread += ProcessUpgradesQueue;
    }
};
