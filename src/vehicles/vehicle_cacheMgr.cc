#include "vehicle_cacheMgr.hh"
#include <common/common.hh>
#include "common/logger.hh"
#include <chrono>
#include <CModelInfo.hh>
#include <CStreaming.hh>
#include <cstdio>
#include <stdio.h>

const uint32_t CacheVersion = 1;

/*******************************************************/
bool
VehicleModelInfoCacheMgr::WriteCacheFile ()
{
    FILE *cacheFile
        = Rainbomizer::Common::GetRainbomizerDataFile ("VehInfo.cache", "wb");

    if (!cacheFile)
        return false;

    auto &hashes = Rainbomizer::Common::GetVehicleHashes ();
    fwrite (&CacheVersion, 4, 1, cacheFile);

    for (auto hash : hashes)
        {
            fwrite (&hash, 4, 1, cacheFile);
            fwrite (&mBoundsCache[hash], sizeof (Vector3), 1, cacheFile);
        }

    fclose (cacheFile);
    return true;
}

/*******************************************************/
void
VehicleModelInfoCacheMgr::GenerateCache ()
{
    auto  timestamp = clock ();
    auto &hashes    = Rainbomizer::Common::GetVehicleHashes ();

    for (auto hash : hashes)
        {
            uint32_t           modelId;
            CVehicleModelInfo *modelInfo
                = CStreaming::GetModelAndIndexByHash<CVehicleModelInfo> (
                    hash, modelId);

            if (!modelInfo || mBoundsCache.count (hash))
                continue;

            bool deleteModel = false;
            if (!CStreaming::HasModelLoaded (modelId))
                {
                    CStreaming::RequestModel (modelId, 4);
                    CStreaming::LoadAllObjects (true);
                    deleteModel = true;
                }

            // Set bounds for the vehicle
            auto [bounds, added]
                = mBoundsCache.try_emplace (hash, modelInfo->m_vecMax
                                                      - modelInfo->m_vecMin);

            Rainbomizer::Logger::LogMessage (
                "Initialised bounds for vehicle %x: %f %f %f", hash,
                bounds->second.x, bounds->second.y, bounds->second.z);

            if (deleteModel)
                CStreaming::DeleteModel (modelId);
        }

    Rainbomizer::Logger::LogMessage (
        "Cached model info in %.2f seconds",
        1.0f * (static_cast<float> (clock () - timestamp) / CLOCKS_PER_SEC));

    WriteCacheFile ();
}

/*******************************************************/
bool
VehicleModelInfoCacheMgr::ReadCacheFile (FILE *file)
{
    uint32_t cacheVersion = 0;
    if (fread (&cacheVersion, 4, 1, file))
        {
            if (cacheVersion == CacheVersion)
                {
                    Vector3  bounds;
                    uint32_t hash;
                    while (fread (&hash, 4, 1, file)
                           && fread (&bounds, sizeof (Vector3), 1, file))
                        mBoundsCache[hash] = bounds;

                    return mBoundsCache.size ()
                           == Rainbomizer::Common::GetVehicleHashes ().size ();
                }
        }
    Rainbomizer::Logger::LogMessage (
        "Failed to read vehicle model info cache, regenerating.");
    return false;
}

/*******************************************************/
void
VehicleModelInfoCacheMgr::InitCache ()
{
    mBoundsCache.clear ();
    FILE *cacheFile
        = Rainbomizer::Common::GetRainbomizerDataFile ("VehInfo.cache", "rb");

    if (!cacheFile || !ReadCacheFile (cacheFile))
        GenerateCache ();

    fclose (cacheFile);
}

/*******************************************************/
const Vector3 &
VehicleModelInfoCacheMgr::GetVehicleBounds (uint32_t vehicle)
{
    if (mBoundsCache.count (vehicle))
        return mBoundsCache[vehicle];

    Rainbomizer::Logger::LogMessage (
        "Vehicle %x not in cache, regenerating cache", vehicle);

    GenerateCache ();

    // Return the generated bounds or 0,0,0 in case the bounds thing failed.
    return mBoundsCache[vehicle];
}
