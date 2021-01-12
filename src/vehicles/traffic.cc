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

class CDispatchService;

bool (*CDispatchService_GetVehicleSetModels98c) (CDispatchService *, uint32_t *,
                                                 uint32_t *, uint32_t *,
                                                 int32_t);

class TrafficRandomizer
{
    /*******************************************************/
    static bool
    RandomizeCarToSpawn (uint32_t *modelId, uint32_t *spawnList, Vector3 *pos,
                         bool param_4)
    {
        *modelId = GetRandomLoadedVehIndex ();
        if (*modelId == -1u)
            *modelId = 65535;

        return ((*modelId & 65535) == 65535);
    }

    /*******************************************************/
    static uint32_t __fastcall RandomizeCarToLoad (CStreaming *inst)
    {
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

    /*******************************************************/
    static bool
    RandomizeDispatchVehicleSet (CDispatchService *service, uint32_t *outVeh,
                                 uint32_t *outPed, uint32_t *outObj, int32_t p5)
    {
        bool ret = CDispatchService_GetVehicleSetModels98c (service, outVeh,
                                                            outPed, outObj, p5);

        uint32_t bmxIndex;
        CStreaming::GetModelAndIndexByHash ("bmx"_joaat, bmxIndex);

        if (!CStreaming::HasModelLoaded (bmxIndex))
            {
                CStreaming::RequestModel (bmxIndex, 0);
                CStreaming::LoadAllObjects (false);
            }

        // CVehicleModelInfo *model
        //     = CStreaming::GetModelByIndex<CVehicleModelInfo> (*outVeh &
        //     0xFFFF);

        // Rainbomizer::Logger::LogMessage ("Dispatch spawning: %x: %s",
        //                                  model->m_nHash, model->GetGameName
        //                                  ());

        if ((*outVeh & 0xFFFF) != 0xFFFF)
            *outVeh = bmxIndex & 4026531839 | 268369920;

        return ret;
    }

public:
    /*******************************************************/
    TrafficRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("TrafficRandomizer"))
            return;

        InitialiseAllComponents ();
        InitialiseDLCDespawnFix ();

        // Actuall spawning of the vehicle
        MakeJMP64 (hook::get_pattern (
                       "? 8b c4 ? 88 ? ? ? 89 ? ? ? 89 ? ? ? 89 ? ? 55 53"),
                   RandomizeCarToSpawn);

        // To load new vehicles (that would probably not be loaded by the game)
        MakeJMP64 (hook::get_pattern ("? 89 5c ? ? ? 89 6c ? ? ? 89 74 ? ? 57 "
                                      "? 81 ec 30 04 00 00 8a 81 00 45 00 00"),
                   RandomizeCarToLoad);

        // Randomize Dispatch service
        RegisterJmpHook<15> (
            injector::GetBranchDestination (
                hook::get_pattern ("8d ? 58 89 ? ? e8 ? ? ? ? 84 c0 0f 84", 6))
                .get<void> (),
            CDispatchService_GetVehicleSetModels98c,
            RandomizeDispatchVehicleSet);

        injector::MakeNOP (
            hook::get_pattern (
                "74 ? 3b 05 ? ? ? ? 74 ? 3b 05 ? ? ? ? 75 ? ? b0 01", 16),
            2);
    }
} _traf;
