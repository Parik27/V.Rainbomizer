#include "vehicle_common.hh"
#include <CStreaming.hh>
#include <rage.hh>
#include <set>
#include <Utils.hh>
#include <scrThread.hh>
#include "CModelInfo.hh"
#include "common/logger.hh"
#include "common/ysc.hh"
#include "mission/missions_YscUtils.hh"

bool (*scrProgram_InitNativeTablese188) (scrProgram *);

/*******************************************************/
std::set<uint32_t>
VehicleRandomizerHelper::GetLoadedVehSet ()
{
    std::set<uint32_t> cars;
    auto               groups = CStreaming::GetModelSets();

    groups->mAppropriateCarsSet.for_each (
        [&cars] (int val) { cars.insert (val); });
    groups->mInappropriateCarsSet.for_each (
        [&cars] (int val) { cars.insert (val); });
    groups->mBoatsSet.for_each ([&cars] (int val) { cars.insert (val); });
    groups->mSpecialVehiclesSet.for_each ([&] (int val) { cars.insert (val); });
    groups->mVehiclesSet6.for_each ([&] (int val) { cars.insert (val); });

    return cars;
}

/*******************************************************/
void
VehicleRandomizerHelper::AdjustSetBasedOnSettings (std::set<uint32_t> &set)
{
    if (!Settings::Trains)
        RemoveVehicleTypesFromSet<"VEHICLE_TYPE_TRAIN"_joaat> (set);

    if (!Settings::Boats)
        RemoveVehicleTypesFromSet<"VEHICLE_TYPE_BOAT"_joaat> (set);

    if (!Settings::Helis)
        RemoveVehicleTypesFromSet<"VEHICLE_TYPE_HELI"_joaat,
                                  "VEHICLE_TYPE_BLIMP"_joaat> (set);

    if (!Settings::Planes)
        RemoveVehicleTypesFromSet<"VEHICLE_TYPE_PLANE"_joaat> (set);

    for (auto hash : Settings::Disabled)
        set.erase (CStreaming::GetModelIndex (hash));

    if (Settings::DisabledPtr)
        for (auto hash : *Settings::DisabledPtr)
            set.erase (CStreaming::GetModelIndex (hash));

    Settings::Reset ();
}

/*******************************************************/
uint32_t
VehicleRandomizerHelper::GetRandomLoadedVehIndex (uint32_t *outNum)
{
    std::set<uint32_t> cars = GetLoadedVehSet ();
    AdjustSetBasedOnSettings (cars);

    if (outNum)
        *outNum = static_cast<uint32_t> (cars.size ());

    if (cars.size () < 1)
        return -1;

    uint32_t vehicle = GetRandomElement (cars);

    // Vehicle should always be loaded if they're in the loaded groups, but
    // just in case
    if (CStreaming::HasModelLoaded (vehicle))
        return vehicle;

    return -1;
}

/*******************************************************/
void
VehicleRandomizerHelper::InitialiseDLCDespawnFix ()
{
    static bool bFixInitialised = false;
    if (std::exchange (bFixInitialised, true))
        return;

    // shop_controller handles despawning DLC vehicles. This edit overrides the
    // branch by NOPing the JZ instruction that makes the code jump to the
    // despawning code.

    // ENTER 01 08 00 ?
    // LOCAL_U8_LOAD 00
    // CALL ? ? ? (<-- should not execute despawning code)
    // INOT
    // JZ ? ? (<-- jumps to the despawning code)
    // LEAVE 01 00

    YscCodeEdits::Add ("DLC Despawn Fix", [] (YscUtilsOps &ops) -> bool {
        if (!ops.IsAnyOf ("shop_controller"_joaat))
            return false;

        ops.Init ("2d ? ? 00 ? 38 00 5d ? ? ? 06 56 ? ? 2e 01 00");
        ops.NOP (/*Offset=*/12, 3);

        return true;
    });
}
