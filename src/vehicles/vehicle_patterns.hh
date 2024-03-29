#pragma once

#include <CMath.hh>
#include <cstdint>
#include <string>
#include <vector>

/*******************************************************/
/* Class to handle a single script vehicle pattern.    */
/*******************************************************/
class ScriptVehiclePattern
{
    struct VehicleTypes
    {
        bool Cars : 1;
        bool Bikes : 1;
        bool Bicycles : 1;
        bool Quadbikes : 1;
        bool Planes : 1;
        bool Helicopters : 1;
        bool Blimps : 1;
        bool Boats : 1;
        bool Submarines : 1;
        bool SubmarineCars : 1;
        bool Trains : 1;
        bool Trailers : 1;

        bool GetValue (uint32_t type);

    } mAllowedTypes, mMovedTypes;

    struct
    {
        bool Flying : 1;
        bool CanTow : 1;
        bool NoRotors : 1;
        bool NoWait : 1;

        uint32_t AttachVehicle = 0; // hash
    } mFlags;

    Vector3        m_vecBounds      = {0.0, 0.0, 0.0};
    Vector3        m_vecMovedCoords = {0.0, 0.0, 0.0};
    Vector3_t<int> m_vecCoordsCheck = {0, 0, 0};

    uint32_t m_nSeatCheck       = 0;
    uint32_t m_nOriginalVehicle = 0;

    bool                  m_bCached = false;
    std::vector<uint32_t> m_aCache;

public:
    /*******************************************************/
    void
    SetBoundsCheck (const Vector3 &bounds)
    {
        m_bCached   = false;
        m_vecBounds = bounds;
    }

    void
    SetSeatsCheck (uint32_t seats)
    {
        m_bCached    = false;
        m_nSeatCheck = seats;
    }
    void
    SetAllowedTypes (VehicleTypes types)
    {
        m_bCached     = false;
        mAllowedTypes = types;
    }
    void
    SetMovedTypes (VehicleTypes types)
    {
        m_bCached   = false;
        mMovedTypes = types;
    }

    void
    SetOriginalVehicle (uint32_t veh)
    {
        m_bCached          = false;
        m_nOriginalVehicle = veh;
    }

    uint32_t
    GetOriginalVehicle () const
    {
        return m_nOriginalVehicle;
    }

    void
    SetMovedCoordinates (const Vector3 &moveCoords)
    {
        m_bCached        = false;
        m_vecMovedCoords = moveCoords;
    }
    const Vector3 &
    GetMovedCoordinates () const
    {
        return m_vecMovedCoords;
    }
    uint32_t
    GetNumVehicles () const
    {
        return static_cast<uint32_t> (m_aCache.size ());
    }

    auto
    GetFlags () const
    {
        return mFlags;
    }

    bool DoesVehicleMatchPattern (uint32_t hash);
    void Cache ();

    uint32_t GetRandom (Vector3 &pos);
    uint32_t GetRandomLoaded (Vector3 &pos);

    // Reads a flag string in the format "flag=value" or "flag" for just bools
    void ReadFlag (const std::string &flag);

    // Reads a list of flags delimited by a '+'
    void ParseFlags (const std::string &flags);

    bool MatchVehicle (uint32_t hash, const Vector3 &coords);

    ScriptVehiclePattern () : mFlags{false, false, false, false, 0} {}
};
