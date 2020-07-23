#include "vehicle_cacheMgr.hh"
#include "common/common.hh"
#include "common/logger.hh"
#include <chrono>
#include <CModelInfo.hh>
#include <CStreaming.hh>
#include <cstdio>

const uint32_t CacheVersion = 1;

/*******************************************************/
void
VehicleModelInfoCacheMgr::GenerateCache ()
{
    auto timestamp = clock ();
    auto& hashes    = Rainbomizer::Common::GetVehicleHashes ();

    FILE *cacheFile
        = Rainbomizer::Common::GetRainbomizerDataFile ("VehInfo.cache", "wb");

    fwrite (&CacheVersion, 4, 1, cacheFile);
    for (auto hash : hashes)
        {
            uint32_t           modelId;
            CVehicleModelInfo *modelInfo
                = CStreaming::GetModelAndIndexByHash<CVehicleModelInfo> (
                    hash, modelId);

            if (!modelInfo)
                continue;

            CStreaming::RequestModel (modelId, 4);
            CStreaming::LoadAllObjects (true);

            mBoundsCache[hash] = modelInfo->m_vecMax - modelInfo->m_vecMin;
            fwrite (&hash, 4, 1, cacheFile);
            fwrite (&mBoundsCache[hash], sizeof (Vector3), 1, cacheFile);
        }

    fclose (cacheFile);

    Rainbomizer::Logger::LogMessage ("Cached model info in %.2f seconds",
                                     1.0f * (clock () - timestamp)
                                         / CLOCKS_PER_SEC);
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
        return GenerateCache ();

    for (const auto &i : mBoundsCache)
        Rainbomizer::Logger::LogMessage ("%x: %f %f %f", i.first, i.second.x,
                                         i.second.y, i.second.z);
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
    return GetVehicleBounds (vehicle);
}

std::unique_ptr<VehicleModelInfoCacheMgr> VehicleModelInfoCacheMgr::sm_Instance;
