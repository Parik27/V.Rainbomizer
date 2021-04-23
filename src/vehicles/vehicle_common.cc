#include "vehicle_common.hh"
#include <CStreaming.hh>
#include <rage.hh>
#include <set>
#include <Utils.hh>
#include <scrThread.hh>
#include "CModelInfo.hh"
#include "common/logger.hh"

bool (*scrProgram_InitNativeTablese188) (scrProgram *);

/*******************************************************/
std::set<uint32_t>
VehicleRandomizerHelper::GetLoadedVehSet ()
{
    std::set<uint32_t> cars;
    auto               groups = CStreaming::sm_Instance;

    groups->mAppropriateCarsSet.for_each (
        [&cars] (int val) { cars.insert (val); });
    groups->mInappropriateCarsSet.for_each (
        [&cars] (int val) { cars.insert (val); });
    groups->mBoatsSet.for_each ([&cars] (int val) { cars.insert (val); });
    groups->mSpecialVehiclesSet.for_each ([&] (int val) { cars.insert (val); });

    return cars;
}

/*******************************************************/
uint32_t
VehicleRandomizerHelper::GetRandomLoadedVehIndex (uint32_t *outNum, bool trains)
{
    std::set<uint32_t> cars = GetLoadedVehSet ();
    if (!trains)
        RemoveVehicleTypesFromSet<"VEHICLE_TYPE_TRAIN"_joaat> (cars);

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
bool
VehicleRandomizerHelper::ApplyDLCDespawnFix (scrProgram *program)
{
    bool ret = scrProgram_InitNativeTablese188 (program);

    if (program->m_nScriptHash == "shop_controller"_joaat)
        {
            // This script handles despawning of DLC vehicles. This finds the
            // function that does that and overrides the branch to make the
            // checks not happen.

            program->ForEachCodePage ([] (int, uint8_t *block, size_t size) {
                // ENTER 01 08 00 ?
                // LOCAL_U8_LOAD 00
                // CALL ? ? ? (<-- should not execute despawning code)
                // INOT
                // JZ ? ? (<-- jumps to the despawning code)
                // LEAVE 01 00

                auto pattern = hook::make_range_pattern (
                    uintptr_t (block), uintptr_t (block) + size,
                    "2d 01 08 00 ? 38 00 5d ? ? ? 06 56 ? ? 2e 01 00");

                pattern.for_each_result ([] (hook::pattern_match match) {
                    // NOPs the JZ instruction
                    // JZ ? ? => NOP NOP NOP
                    *match.get<uint16_t> (12) = 0;
                    *match.get<uint8_t> (14)  = 0;

                    Rainbomizer::Logger::LogMessage ("Applied DLC Despawn fix");

                    return true;
                });
            });
        }
    return ret;
}

/*******************************************************/
void
VehicleRandomizerHelper::InitialiseDLCDespawnFix ()
{
    static bool bFixInitialised = false;
    if (!std::exchange (bFixInitialised, true))
        RegisterHook ("8b cb e8 ? ? ? ? 8b 43 70 ? 03 c4 a9 00 c0 ff ff", 2,
                      scrProgram_InitNativeTablese188, ApplyDLCDespawnFix);
}
