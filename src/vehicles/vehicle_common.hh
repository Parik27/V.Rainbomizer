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
    inline static bool
    IsVehicleAnyOfType (uint32_t idx)
    {
        bool     ret     = false;
        uint32_t vehType = CStreaming::GetModelByIndex<CVehicleModelInfo> (idx)
                               ->GetVehicleType ();
        (..., (ret = vehType == Hashes ? true : ret));

        return ret;
    }

    /*******************************************************/
    template <uint32_t... Hashes>
    inline static void
    RemoveVehicleTypesFromSet (std::set<uint32_t> &set)
    {        
        for (auto it = set.begin (), end = set.end (); it != end;)
            {
                if (IsVehicleAnyOfType<Hashes...>(*it))
                    it = set.erase(it);
                else
                    ++it;
            }
    }

    /*******************************************************/
    template <uint32_t... Hashes>
    inline static void
    RemoveOtherVehicleTypesFromSet (std::set<uint32_t> &set)
    {
        for (auto it = set.begin (), end = set.end (); it != end;)
            {
                if (!IsVehicleAnyOfType<Hashes...>(*it))
                    it = set.erase(it);
                else
                    ++it;
            }
    }

    static uint32_t GetRandomLoadedVehIndex (uint32_t *outNum = nullptr,
                                             bool      trains = false);

    static void InitialiseDLCDespawnFix ();
};
