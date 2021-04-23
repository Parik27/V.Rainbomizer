#pragma once

#include "CStreaming.hh"
#include <cstdint>
#include <set>

struct scrProgram;

class VehicleRandomizerHelper
{
    static bool ApplyDLCDespawnFix (scrProgram *program);

public:
    static std::set<uint32_t> GetLoadedVehSet ();

    /*******************************************************/
    template <uint32_t... Hashes>
    inline static void
    RemoveVehicleTypesFromSet (std::set<uint32_t> &set)
    {
        set.erase (std::remove_if (set.begin (), set.end (), [] (uint32_t id) {
            bool     ret = true;
            uint32_t vehType
                = CStreaming::GetModelByIndex<CVehicleModelInfo> (id)
                      ->GetVehicleType ();
            (..., (ret = vehType == Hashes ? true : ret));
        }));
    }

    /*******************************************************/
    template <uint32_t... Hashes>
    inline static void
    RemoveOtherVehicleTypesFromSet (std::set<uint32_t> &set)
    {
        set.erase (std::remove_if (set.begin (), set.end (), [] (uint32_t id) {
            bool     ret = true;
            uint32_t vehType
                = CStreaming::GetModelByIndex<CVehicleModelInfo> (id)
                      ->GetVehicleType ();
            (..., (ret = vehType != Hashes ? true : ret));
        }));
    }

    static uint32_t GetRandomLoadedVehIndex (uint32_t *outNum = nullptr,
                                             bool      trains = false);

    static void InitialiseDLCDespawnFix ();
};
