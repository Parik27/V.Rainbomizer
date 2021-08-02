#pragma once

#include "CStreaming.hh"
#include <cstdint>
#include <set>
#include <vector>

struct scrProgram;

class VehicleRandomizerHelper
{
public:
    struct Settings
    {
        inline static bool Trains = false;
        inline static bool Helis  = true;
        inline static bool Planes = true;
        inline static bool Boats  = true;

        inline static std::vector<uint32_t>        Disabled;
        inline static const std::vector<uint32_t> *DisabledPtr = nullptr;

        static void
        Reset ()
        {
            Trains = false;
            Helis  = true;
            Planes = true;
            Boats  = true;

            Disabled.clear ();

            DisabledPtr = nullptr;
        }
    };

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
                if (IsVehicleAnyOfType<Hashes...> (*it))
                    it = set.erase (it);
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
                if (!IsVehicleAnyOfType<Hashes...> (*it))
                    it = set.erase (it);
                else
                    ++it;
            }
    }

    static void AdjustSetBasedOnSettings (std::set<uint32_t> &set);

    static uint32_t GetRandomLoadedVehIndex (uint32_t *outNum = nullptr);

    static void InitialiseDLCDespawnFix ();
};
