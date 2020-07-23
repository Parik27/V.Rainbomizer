#include <Utils.hh>
#include "common/logger.hh"
#include <CStreaming.hh>
#include <rage.hh>
#include <Natives.hh>
#include <CMath.hh>
#include "common/common.hh"
#include <set>
#include "common/config.hh"
#include "vehicle_common.hh"

struct Vector3;

class TrafficRandomizer
{
    /*******************************************************/
    static bool
    RandomizeCarToSpawn (uint32_t *modelId, uint32_t *spawnList, Vector3 *pos,
                         bool param_4)
    {
        *modelId = GetRandomLoadedVehIndex ();
        return ((*modelId & 65536) == 65535);
    }

    /*******************************************************/
    static uint32_t __fastcall RandomizeCarToLoad (CStreaming *inst)
    {
        auto &vehicles = Rainbomizer::Common::GetVehicleHashes ();
        for (int i = 0; i < 16; i++)
            {
                uint32_t vehicle = vehicles[RandomInt (vehicles.size () - 1)];
                uint32_t vehicleIndex;
                auto     model
                    = CStreaming::GetModelAndIndexByHash<CVehicleModelInfo> (
                        vehicle, vehicleIndex);

                if (CStreaming::HasModelLoaded (vehicleIndex)
                    && model->GetVehicleType () != "VEHICLE_TYPE_TRAIN"_joaat)
                    continue;

                return vehicleIndex;
            }
        return 65535; // -1/uint16_t
    }

public:
    /*******************************************************/
    TrafficRandomizer ()
    {
        if (!ConfigManager::GetConfigs().traffic.enabled)
            return;
        
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
