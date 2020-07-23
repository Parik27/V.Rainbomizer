#include "vehicle_common.hh"
#include <CStreaming.hh>
#include <rage.hh>
#include <set>
#include <Utils.hh>

/*******************************************************/
uint32_t
GetRandomLoadedVehIndex (uint32_t *outNum, bool trains)
{
    std::set<uint32_t> cars;
    auto               groups = CStreaming::sm_Instance;

    groups->mAppropriateCarsSet.for_each (
        [&cars] (int val) { cars.insert (val); });
    groups->mInappropriateCarsSet.for_each (
        [&cars] (int val) { cars.insert (val); });
    groups->mBoatsSet.for_each ([&cars] (int val) { cars.insert (val); });

    groups->mSpecialVehiclesSet.for_each ([&] (int val) {
        if (trains
            || CStreaming::GetModelByIndex<CVehicleModelInfo> (val)
                       ->GetVehicleType ()
                   != "VEHICLE_TYPE_TRAIN"_joaat)
            cars.insert (val);
    });

    if (outNum)
        *outNum = cars.size();
    
    if (cars.size () < 1)
        return -1;

    auto it = cars.begin ();
    std::advance (it, RandomInt (cars.size () - 1));

    // Vehicle should always be loaded if they're in the loaded groups, but
    // just in case
    if (CStreaming::HasModelLoaded (*it))
        return *it;

    return -1;
}
