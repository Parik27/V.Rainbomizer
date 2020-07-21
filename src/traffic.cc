#include <Utils.hh>
#include "logger.hh"
#include <CStreaming.hh>
#include <rage.hh>
#include <Natives.hh>
#include <CMath.hh>
#include "common.hh"
#include <set>

struct Vector3;

class TrafficRandomizer
{
    /*******************************************************/
    static bool
    RandomizeCarToSpawn (uint32_t *modelId, uint32_t *spawnList, Vector3 *pos,
                         bool param_4)
    {
        std::set<uint32_t> cars;
        auto groups = CStreaming::sm_Instance;
        
        groups->mAppropriateCarsSet.for_each (
            [&cars] (int val) { cars.insert (val); });
        groups->mInappropriateCarsSet.for_each (
            [&cars] (int val) { cars.insert (val); });
        groups->mBoatsSet.for_each ([&cars] (int val) { cars.insert (val); });

        groups->mSpecialVehiclesSet.for_each ([&cars] (int val) {
            if (CStreaming::GetModelByIndex<CVehicleModelInfo> (val)
                    ->GetVehicleType ()
                != "VEHICLE_TYPE_TRAIN"_joaat)
                cars.insert (val);
        });

        *modelId = 65535;
        if (cars.size () < 1)
            return false;

        auto it = cars.begin ();
        std::advance (it, RandomInt (cars.size () - 1));

        // Vehicle should always be loaded if they're in the loaded groups, but
        // just in case
        if (CStreaming::HasModelLoaded(*it))
            *modelId = *it;

        return *modelId == 65535;
    }

    /*******************************************************/
    static uint32_t __fastcall RandomizeCarToLoad (CStreaming *inst)
    {
        auto &vehicles = Rainbomizer::Common::GetVehicleHashes ();
        for (int i = 0; i < 16; i++)
            {
                uint32_t vehicle = vehicles[RandomInt (vehicles.size () - 1)];
                uint32_t vehicleIndex = CStreaming::GetModelIndex (vehicle);

                if (CStreaming::HasModelLoaded (vehicleIndex))
                    continue;

                return vehicleIndex;
            }
        return 65535; // -1/uint16_t
    }

public:
    /*******************************************************/
    TrafficRandomizer ()
    {
        InitialiseAllComponents ();

        // Actuall spawning of the vehicle
        MakeJMP64 (hook::get_pattern (
                       "? 8b c4 ? 88 ? ? ? 89 ? ? ? 89 ? ? ? 89 ? ? 55 53"),
                   RandomizeCarToSpawn);

        // To load new vehicles (that would probably not be loaded by the game)
        MakeJMP64 (hook::get_pattern ("? 89 5c ? ? ? 89 6c ? ? ? 89 74 ? ? 57 "
                                      "? 81 ec 30 04 00 00 8a 81 00 45 00 00 "),
                   RandomizeCarToLoad);

    }
} _traf;
