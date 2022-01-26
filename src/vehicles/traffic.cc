#include <Utils.hh>
#include "common/logger.hh"
#include <CStreaming.hh>
#include <rage.hh>
#include <Natives.hh>
#include <CMath.hh>
#include "common/common.hh"
#include <set>
#include <stdio.h>
#include <string.h>
#include "common/config.hh"
#include "vehicle_common.hh"

class TrafficRandomizer
{
    /*******************************************************/
    static auto &
    Config ()
    {
        static struct Config
        {
            bool EnablePlanes = true;
            bool EnableHelis  = true;
            bool EnableBoats  = true;

            bool DisableBigVehicles = false;
            bool DisableLoadingTraffic = false;
        } m_Config;

        return m_Config;
    }

    /*******************************************************/
    static std::vector<uint32_t> *
    GetBigVehiclesList ()
    {
        if (!Config ().DisableBigVehicles)
            return nullptr;

        static bool                  bInitialised = false;
        static std::vector<uint32_t> aList;

        if (bInitialised)
            return &aList;

        bInitialised = true;

        FILE *f
            = Rainbomizer::Common::GetRainbomizerDataFile ("BigVehicles.txt");

        if (!f)
            return &aList;

        char line[256] = {0};
        while (fgets (line, 256, f))
            {
                line[strcspn (line, "\n")] = 0;
                aList.push_back (rage::atStringHash (line));
            }

        fclose (f);
        return &aList;
    }

    /*******************************************************/
    static bool
    RandomizeCarToSpawn (uint32_t *modelId, uint32_t *spawnList, Vector3 *pos,
                         bool param_4)
    {
        VehicleRandomizerHelper::Settings::Boats       = Config ().EnableBoats;
        VehicleRandomizerHelper::Settings::Helis       = Config ().EnableHelis;
        VehicleRandomizerHelper::Settings::Planes      = Config ().EnablePlanes;
        VehicleRandomizerHelper::Settings::DisabledPtr = GetBigVehiclesList ();

        *modelId = VehicleRandomizerHelper::GetRandomLoadedVehIndex ();
        if (*modelId == -1u)
            *modelId = 65535;

        return ((*modelId & 65535) == 65535);
    }

    /*******************************************************/
    static uint32_t __fastcall RandomizeCarToLoad (CStreaming *inst)
    {
        if (Config ().DisableLoadingTraffic)
            return 65535;

        auto &vehicles = Rainbomizer::Common::GetVehicleHashes ();
        for (int i = 0; i < 16; i++)
            {
                uint32_t vehicle = GetRandomElement (vehicles);
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
        RB_C_DO_CONFIG ("TrafficRandomizer", EnablePlanes, EnableHelis,
                        EnableBoats, DisableBigVehicles, DisableLoadingTraffic);

        if (!Config ().EnableBoats)
            Rainbomizer::Logger::LogMessage ("Boat hater detected!!");

        InitialiseAllComponents ();
        VehicleRandomizerHelper::InitialiseDLCDespawnFix ();

        // Actuall spawning of the vehicle
        MakeJMP64 (hook::get_pattern (
                       "? 8b c4 ? 88 ? ? ? 89 ? ? ? 89 ? ? ? 89 ? ? 55 53"),
                   RandomizeCarToSpawn);

        // To load new vehicles (that would probably not be loaded by the game)
        MakeJMP64 (hook::get_pattern ("? 89 5c ? ? ? 89 6c ? ? ? 89 74 ? ? 57 "
                                      "? 81 ec 30 04 00 00 8a 81 00 45 00 00"),
                   RandomizeCarToLoad);
    }
} _traf;
