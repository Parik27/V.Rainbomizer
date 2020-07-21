#include "Patterns/Patterns.hh"
#include "CMath.hh"
#include "Utils.hh"
#include "Natives.hh"
#include "CStreaming.hh"
#include "common.hh"
#include "logger.hh"
#include "CEntity.hh"
#include "UtilsHooking.hh"
#include <CMath.hh>
#include <sstream>
#include <rage.hh>
#include <unordered_map>
#include <scrThread.hh>
#include <map>
#include <chrono>
#include <memory>

CEntity *(*CPools__GetAtEntity) (int);

/*******************************************************/
/* Class to handle a vehicle model info cache.         */
/*******************************************************/
class VehicleModelInfoCacheMgr
{
    const uint32_t CacheVersion = 1;
    std::unordered_map<uint32_t, Vector3> mBoundsCache;
    static std::unique_ptr<VehicleModelInfoCacheMgr> sm_Instance;

public:
    /*******************************************************/
    void
    GenerateCache ()
    {
        auto timestamp = clock ();
        auto hashes    = Rainbomizer::Common::GetVehicleHashes ();

        FILE *cacheFile
            = Rainbomizer::Common::GetRainbomizerDataFile ("VehInfo.cache",
                                                           "wb");

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

        Rainbomizer::Logger::LogMessage (
            "Cached model info in %.2f seconds",
            1.0f * (clock () - timestamp) / CLOCKS_PER_SEC);
    }

    /*******************************************************/
    bool
    ReadCacheFile (FILE *file)
    {
        uint32_t cacheVersion = 0;
        if (fread (&cacheVersion, 4, 1, file) == 4)
            {
                if (cacheVersion == CacheVersion)
                    {
                        Vector3  bounds;
                        uint32_t hash;
                        while (fread (&hash, 4, 1, file) == 4
                               && fread (&bounds, sizeof (Vector3), 1, file)
                                      == sizeof (Vector3))
                            mBoundsCache[hash] = bounds;

                        return mBoundsCache.size ()
                               == Rainbomizer::Common::GetVehicleHashes ()
                                      .size ();
                    }
            }
        Rainbomizer::Logger::LogMessage (
            "Failed to read vehicle model info cache, regenerating.");
        return false;
    }

    /*******************************************************/
    void
    InitCache ()
    {
        mBoundsCache.clear ();
        FILE *cacheFile
            = Rainbomizer::Common::GetRainbomizerDataFile ("VehInfo.cache", "rb");

        if (!cacheFile || !ReadCacheFile (cacheFile))
            return GenerateCache ();

        for (const auto &i : mBoundsCache)
            printf ("%x: %f %f %f", i.first, i.second.x, i.second.y,
                    i.second.z);
    }

    /*******************************************************/
    VehicleModelInfoCacheMgr()
    {
        InitCache ();
    }
    
    /*******************************************************/
    const Vector3 &GetVehicleBounds (uint32_t vehicle)
    {
        if (mBoundsCache.count(vehicle))
            return mBoundsCache[vehicle];

        Rainbomizer::Logger::LogMessage (
            "Vehicle %x not in cache, regenerating cache", vehicle);
        
        GenerateCache ();
        return GetVehicleBounds (vehicle);
    }

    /*******************************************************/
    static VehicleModelInfoCacheMgr* GetInstance ()
    {
        if (!sm_Instance)
            sm_Instance = std::make_unique<VehicleModelInfoCacheMgr> ();
        
        return sm_Instance.get();
    }
};

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

        bool
        GetValue (uint32_t type)
        {
            switch (type)
                {
                case "VEHICLE_TYPE_CAR"_joaat: return Cars;
                case "VEHICLE_TYPE_BICYCLE"_joaat: return Bicycles;
                case "VEHICLE_TYPE_BIKE"_joaat: return Bikes;
                case "VEHILE_TYPE_BLIMP"_joaat: return Blimps;
                case "VEHICLE_TYPE_BOAT"_joaat: return Boats;
                case "VEHICLE_TYPE_HELI"_joaat: return Helicopters;
                case "VEHICLE_TYPE_PLANE"_joaat: return Planes;
                case "VEHICLE_TYPE_SUBMARINE"_joaat: return Submarines;
                case "VEHICLE_TYPE_SUBMARINECAR"_joaat: return SubmarineCars;
                case "VEHICLE_TYPE_TRAILER"_joaat: return Trailers;
                case "VEHICLE_TYPE_TRAIN"_joaat: return Trains;
                case "VEHICLE_TYPE_QUADBIKE"_joaat: return Quadbikes;
                default:
                    Rainbomizer::Logger::LogMessage (
                        "Unknown vehicle type: %x, returning VEHICLE_TYPE_CARS",
                        type);
                    return Cars;
                }
        }

    } mAllowedTypes, mMovedTypes;

    struct
    {
        bool     Flying : 1;
        bool     CanTow : 1;
        uint32_t AttachVehicle = 0; // hash
    } mFlags;

    Vector3 m_vecBounds      = {0.0, 0.0, 0.0};
    Vector3 m_vecMovedCoords = {0.0, 0.0, 0.0};

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
    GetOriginalVehicle ()
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
    GetMovedCoordinates ()
    {
        return m_vecMovedCoords;
    }

    /*******************************************************/
    bool
    DoesVehicleMatchPattern (uint32_t hash)
    {
        uint32_t           index = -1;
        CVehicleModelInfo *model
            = CStreaming::GetModelAndIndexByHash<CVehicleModelInfo> (hash,
                                                                     index);

        if (!model)
            return false;

        uint32_t numSeats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS (hash);

        // Seat check
        if (numSeats < m_nSeatCheck)
            return false;

        // Bounds check (check so that cache manager is only initialised if it's
        // needed
        if (m_vecBounds.x > 0.0 || m_vecBounds.y > 0.0 || m_vecBounds.z > 0.0)
            {
                const Vector3 &bounds = VehicleModelInfoCacheMgr::GetInstance ()
                                            ->GetVehicleBounds (hash);

                if (bounds.x > m_vecBounds.x || bounds.y > m_vecBounds.y
                    || bounds.z > m_vecBounds.z)
                    return false;
            }

        if (mFlags.CanTow && hash != "towtruck"_joaat
            && hash != "towtruck2"_joaat)
            return false;

        if (mFlags.AttachVehicle != 0)
            return false;

        // Type check (it has to be both not moved and allowed)
        if (!mAllowedTypes.GetValue (model->GetVehicleType ())
            && !mMovedTypes.GetValue (model->GetVehicleType ()))
            return false;

        return true;
    }

    /*******************************************************/
    void
    Cache ()
    {
        auto &hashes = Rainbomizer::Common::GetVehicleHashes ();
        m_aCache.clear ();

        for (uint32_t i : hashes)
            {
                if (i == m_nOriginalVehicle || DoesVehicleMatchPattern (i))
                    m_aCache.push_back (i);
            }

        if (!m_aCache.size ())
            assert (!"Empty vehicle pattern detected");

        m_bCached = true;
    }

    /*******************************************************/
    uint32_t
    GetRandom (Vector3 &pos)
    {
        if (!m_bCached)
            Cache ();

        uint32_t modelHash = m_aCache[RandomInt (m_aCache.size () - 1)];
        auto     modelInfo
            = CStreaming::GetModelByHash<CVehicleModelInfo> (modelHash);

        if (mMovedTypes.GetValue (modelInfo->GetVehicleType ()))
            pos += GetMovedCoordinates ();

        return modelHash;
    }

    /*******************************************************/
    void
    ReadFlag (const std::string &flag)
    {
        m_bCached = false;

        /*
          xyz - bound checks
          flying - can vehicle fly (to try and include flying cars / bikes)
          can_tow - vehicles that can tow cars with hook
          can_attach - trucks that can attach a trailer
        */
        if (flag == "flying")
            mFlags.Flying = true;

        else if (flag == "can_tow")
            mFlags.CanTow = true;

        else if (flag.find ("can_attach") == 0)
            mFlags.AttachVehicle = rage::atStringHashLowercase (
                flag.substr (sizeof ("can_attach")).c_str ());

        // Bounds
        else if (flag.find ("x=") == 0)
            m_vecBounds.x = std::stof (flag.substr (2));
        else if (flag.find ("y=") == 0)
            m_vecBounds.y = std::stof (flag.substr (2));
        else if (flag.find ("z=") == 0)
            m_vecBounds.z = std::stof (flag.substr (2));
    }

    /*******************************************************/
    void
    ParseFlags (const std::string &flags)
    {
        std::istringstream flagStream (flags);
        std::string        flag = "";

        while (std::getline (flagStream, flag, '+'))
            ReadFlag (flag);
    }
};

class ScriptVehicleRandomizer
{
    static std::unordered_map<uint32_t, std::vector<ScriptVehiclePattern>>
        mPatterns;

    /*******************************************************/
    static void
    InitialisePatterns ()
    {
        static bool mPatternsInitialised = false;
        if (std::exchange (mPatternsInitialised, true))
            return;

        FILE *file = Rainbomizer::Common::GetRainbomizerDataFile (
            "VehiclePatterns.txt");

        if (!file)
            return;

        char line[2048] = {0};
        while (fgets (line, 2048, file))
            {
                if (line[0] == '#' || strlen (line) < 10)
                    continue;

                char threadName[64]  = {0};
                char vehicleName[64] = {0};
                char flags[256]      = {0};
                int  seats           = 0;
                char cars            = 'N';
                char bikes           = 'N';
                char bicycles        = 'N';
                char quadbikes       = 'N';
                char planes          = 'N';
                char helicopters     = 'N';
                char blimps          = 'N';
                char boats           = 'N';
                char submarines      = 'N';
                char submarinecars   = 'N';
                char trains          = 'N';
                char trailers        = 'N';

                Vector3 altCoords = {0.0, 0.0, 0.0};

                sscanf (
                    line,
                    "%s %s %d %c %c %c %c %c %c %c %c %c %c %c %c %s %f %f %f",
                    threadName, vehicleName, &seats, &cars, &bikes, &bicycles,
                    &quadbikes, &planes, &helicopters, &blimps, &boats,
                    &submarines, &submarinecars, &trains, &trailers, flags,
                    &altCoords.x, &altCoords.y, &altCoords.z);

                ScriptVehiclePattern pattern;
                pattern.SetOriginalVehicle (
                    rage::atStringHashLowercase (vehicleName));
                pattern.SetSeatsCheck (seats);

                pattern.SetAllowedTypes (
                    {cars == 'Y', bikes == 'Y', bicycles == 'Y',
                     quadbikes == 'Y', planes == 'Y', helicopters == 'Y',
                     blimps == 'Y', boats == 'Y', submarines == 'Y',
                     submarinecars == 'Y', trains == 'Y', trailers == 'Y'});

                pattern.SetMovedTypes (
                    {cars == 'C', bikes == 'C', bicycles == 'C',
                     quadbikes == 'C', planes == 'C', helicopters == 'C',
                     blimps == 'C', boats == 'C', submarines == 'C',
                     submarinecars == 'C', trains == 'C', trailers == 'C'});

                pattern.SetMovedCoordinates (altCoords);
                pattern.ParseFlags (flags);

                pattern.Cache ();
                mPatterns[rage::atStringHashLowercase (threadName)].push_back (
                    std::move (pattern));
            }
    }

    /*******************************************************/
    static uint32_t
    GetRandomHashForVehicle (uint32_t hash, Vector3 &coords)
    {
        uint32_t scrHash
            = scrThread::GetActiveThread ()->m_Context.m_nScriptHash;

        InitialisePatterns ();

        // Return a truly random vehicle
        if (mPatterns.count (scrHash))
            {
                for (auto &i : mPatterns[scrHash])
                    {
                        if (i.GetOriginalVehicle () == hash)
                            return i.GetRandom (coords);
                    }
            }

        // Return a truly random vehicle on no match
        auto indices = Rainbomizer::Common::GetVehicleHashes ();
        return indices[RandomInt (
            Rainbomizer::Common::GetVehicleHashes ().size ())];
    }

    /*******************************************************/
    static void
    RandomizeScriptVehicle (uint32_t &hash, Vector3_native *coords,
                            double heading, bool isNetwork,
                            bool thisScriptCheck)
    {
        uint32_t originalHash = hash;

        // New vehicle
        Vector3 pos = {coords->x, coords->y, coords->z};
        hash        = GetRandomHashForVehicle (hash, pos);

        uint32_t index = CStreaming::GetModelIndex (hash);
        CStreaming::RequestModel (index, 0);

        // TODO: Is there a way to just load the vehicle?
        LOAD_ALL_OBJECTS_NOW ();

        // Set hash back to original if the vehicle failed to load else update
        // coords to alternative coords
        if (!CStreaming::HasModelLoaded (index))
            {
                hash = originalHash;
            }
        else
            {
                coords->x = pos.x;
                coords->y = pos.y;
                coords->z = pos.z;
            }
    }

    /*******************************************************/
    static void
    InitialiseRandomVehiclesHook ()
    {
        ReplaceJmpHook__fastcall<0x7c1c0, int, uint32_t, Vector3_native *,
                                 double, bool, bool> (
            hook::get_pattern ("8b ec ? 83 ec 50 f3 0f 10 02 f3 0f 10 4a 08 ",
                               -17),
            RandomizeScriptVehicle, CALLBACK_ORDER_BEFORE)
            .Activate ();
    }

public:
    /*******************************************************/
    ScriptVehicleRandomizer ()
    {
        InitialiseAllComponents ();
        InitialiseRandomVehiclesHook ();
    }
};

std::unordered_map<uint32_t, std::vector<ScriptVehiclePattern>>
    ScriptVehicleRandomizer::mPatterns;

std::unique_ptr<VehicleModelInfoCacheMgr> VehicleModelInfoCacheMgr::sm_Instance;

ScriptVehicleRandomizer _scr;
