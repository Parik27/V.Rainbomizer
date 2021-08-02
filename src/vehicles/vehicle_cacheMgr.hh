#pragma once

#include <cstdint>
#include <unordered_map>
#include <memory>
#include <CMath.hh>

/*******************************************************/
/* Class to handle a vehicle model info cache.         */
/*******************************************************/
class VehicleModelInfoCacheMgr
{
    std::unordered_map<uint32_t, Vector3>                   mBoundsCache;
    static inline std::unique_ptr<VehicleModelInfoCacheMgr> sm_Instance;

public:
    void GenerateCache ();
    bool ReadCacheFile (FILE *file);
    bool WriteCacheFile ();
    void InitCache ();

    VehicleModelInfoCacheMgr () { InitCache (); }
    const Vector3 &GetVehicleBounds (uint32_t vehicle);

    /*******************************************************/
    static VehicleModelInfoCacheMgr *
    GetInstance ()
    {
        if (!sm_Instance)
            sm_Instance = std::make_unique<VehicleModelInfoCacheMgr> ();

        return sm_Instance.get ();
    }
};
