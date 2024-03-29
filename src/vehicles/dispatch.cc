#include "CModelInfo.hh"
#include "Patterns/Patterns.hh"
#include "common/common.hh"
#include "injector/injector.hpp"
#include "vehicles/vehicle_common.hh"
#include <Utils.hh>
#include "common/config.hh"
#include <CStreaming.hh>
#include <cstdint>
#include <rage.hh>
#include <CDispatchService.hh>
#include <vector>
#include "vehicle_common.hh"

bool (*CDispatchService_GetVehicleSetModels98c) (CDispatchService *, uint32_t *,
                                                 uint32_t *, uint32_t *,
                                                 int32_t);

class DispatchRandomizer
{
    /*******************************************************/
    static void
    StoreVehicleIndex (uint32_t idx, uint32_t *outVeh)
    {
        if ((*outVeh & 0xFFFF) != 0xFFFF)
            *outVeh = idx & 4026531839 | 268369920;
    }

    /*******************************************************/
    static uint32_t
    GetRandomVehicleForPoliceAutomobile ()
    {
        auto vehs = VehicleRandomizerHelper::GetLoadedVehSet ();

        VehicleRandomizerHelper::RemoveVehicleTypesFromSet<
            "VEHICLE_TYPE_HELI"_joaat, "VEHICLE_TYPE_BOAT"_joaat,
            "VEHICLE_TYPE_TRAIN"_joaat, "VEHICLE_TYPE_TRAILER"_joaat> (vehs);

        if (vehs.size ())
            return GetRandomElement (vehs);

        return ~0;
    }

    /*******************************************************/
    static uint32_t
    GetRandomPoliceHelicopter ()
    {
        auto vehs = VehicleRandomizerHelper::GetLoadedVehSet ();

        VehicleRandomizerHelper::RemoveOtherVehicleTypesFromSet<
            "VEHICLE_TYPE_HELI"_joaat> (vehs);

        if (vehs.size ())
            return GetRandomElement (vehs);

        return ~0;
    }

    /*******************************************************/
    static uint32_t
    GetRandomVehicle ()
    {
        return VehicleRandomizerHelper::GetRandomLoadedVehIndex ();
    }

    /*******************************************************/
    static uint32_t
    GetRandomBoat ()
    {
        auto vehs = VehicleRandomizerHelper::GetLoadedVehSet ();

        VehicleRandomizerHelper::RemoveOtherVehicleTypesFromSet<
            "VEHICLE_TYPE_BOAT"_joaat, "VEHICLE_TYPE_SUBMARINE"_joaat,
            "VEHICLE_TYPE_SUBMARINECAR"_joaat> (vehs);

        if (vehs.size ())
            return GetRandomElement (vehs);

        return ~0;
    }

    /*******************************************************/
    static uint32_t
    GetVehicleForDispatchService (CDispatchService *service)
    {
        switch (service->eId)
            {

            case eDispatchId::DT_PoliceAutomobile:
            case eDispatchId::DT_ArmyVehicle:
            case eDispatchId::DT_SwatAutomobile:
            case eDispatchId::DT_PoliceRiders:
            case eDispatchId::DT_PoliceVehicleRequest:
                return GetRandomVehicleForPoliceAutomobile ();

            case eDispatchId::DT_PoliceHelicopter:
            case eDispatchId::DT_SwatHelicopter:
                return GetRandomPoliceHelicopter ();

            case eDispatchId::DT_PoliceBoat: return GetRandomBoat (); break;

            case eDispatchId::DT_FireDepartment:
            case eDispatchId::DT_AmbulanceDepartment:
            case eDispatchId::DT_Gangs:
            case eDispatchId::DT_PoliceRoadBlock:
            default: return GetRandomVehicle ();
            }

        return -1;
    }

    /*******************************************************/
    static bool
    RandomizeDispatchVehicleSet (CDispatchService *service, uint32_t *outVeh,
                                 uint32_t *outPed, uint32_t *outObj, int32_t p5)
    {
        bool ret = CDispatchService_GetVehicleSetModels98c (service, outVeh,
                                                            outPed, outObj, p5);

        StoreVehicleIndex (GetVehicleForDispatchService (service), outVeh);
        return ret;
    }

public:
    /*******************************************************/
    DispatchRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("DispatchRandomizer"))
            return;

        InitialiseAllComponents ();
        VehicleRandomizerHelper::InitialiseDLCDespawnFix ();

        // Randomize Dispatch service
        RegisterJmpHook<15> (
            injector::GetBranchDestination (
                hook::get_pattern ("8d ? 58 89 ? ? e8 ? ? ? ? 84 c0 0f 84", 6))
                .get<void> (),
            CDispatchService_GetVehicleSetModels98c,
            RandomizeDispatchVehicleSet);

        // Disable SWAT vehicle check (so that you can spawn SWAT on non-normal
        // vehicles)
        injector::MakeNOP (
            hook::get_pattern (
                "74 ? 3b 05 ? ? ? ? 74 ? 3b 05 ? ? ? ? 75 ? ? b0 01", 16),
            2);

        // Disable vehicle law enforcement check to allow peds to be spawned on
        // the vehicles.
        injector::WriteMemory<uint8_t> (
            hook::get_pattern (
                "8b ? ? ? ? ? 41 8b f1 45 8a d0 c1 e8 1f 40 8a ea 41 84 c4 75",
                21),
            0xeb);
    }

} disp;
