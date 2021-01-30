#include "Patterns/Patterns.hh"
#include "CMath.hh"
#include "Utils.hh"
#include "Natives.hh"
#include "CStreaming.hh"
#include "common/common.hh"
#include "common/logger.hh"
#include "CEntity.hh"
#include "UtilsHooking.hh"
#include <CMath.hh>
#include <rage.hh>
#include <unordered_map>
#include <scrThread.hh>
#include "vehicle_patterns.hh"
#include "vehicle_common.hh"
#include "common/config.hh"
#include <CPools.hh>


#ifdef ENABLE_DEBUG_SERVER
#include "debug/actions.hh"
#endif

CEntity *(*CPools__GetAtEntity) (int);
bool (*CVehicle__IsVehDriveable4137) (void *, bool, bool, bool);

class ScriptVehicleRandomizer
{
    static inline std::unordered_map<uint32_t,
                                     std::vector<ScriptVehiclePattern>>
        mPatterns;

    static auto &
    Config ()
    {
        static struct Config
        {
            bool LogSpawnedVehicles = false;
        } m_Config;

        return m_Config;
    }

    /*******************************************************/
    static void
    InitialisePatterns ()
    {
        static bool mPatternsInitialised = false;

#ifdef ENABLE_DEBUG_SERVER
        if (ActionsDebugInterface::sm_ReloadPatternsRequested)
            {
                mPatternsInitialised = false;
                mPatterns.clear ();

                ActionsDebugInterface::sm_ReloadPatternsRequested = false;
            }
#endif

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
                pattern.SetOriginalVehicle (rage::atStringHash (vehicleName));
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
                mPatterns[rage::atStringHash (threadName)].push_back (
                    std::move (pattern));
            }
    }

    /*******************************************************/
    static uint32_t
    GetRandomHashForVehicle (uint32_t hash, Vector3 &coords)
    {
        const int MIN_FREE_SLOTS = 25;

        uint32_t scrHash
            = scrThread::GetActiveThread ()->m_Context.m_nScriptHash;

        InitialisePatterns ();

        // Check if VehicleStruct pool is almost full. Return a random loaded
        // vehicle in that case.
        bool returnLoaded
            = CPools::GetVehicleStructPool ()->GetCount () + MIN_FREE_SLOTS
              >= CPools::GetVehicleStructPool ()->m_nMaxElements;

        // Return a truly random vehicle
        if (mPatterns.count (scrHash))
            {
                for (auto &i : mPatterns[scrHash])
                    {
                        if (i.MatchVehicle (hash, coords))
                            return (returnLoaded) ? i.GetRandomLoaded (coords)
                                                  : i.GetRandom (coords);
                    }
            }

        // Return a random vehicle
        {
            uint32_t numRandomLoaded = 0;
            uint32_t randomLoaded
                = GetRandomLoadedVehIndex (&numRandomLoaded, false);
            if (!returnLoaded && numRandomLoaded < 30)
                {
                    const auto &indices
                        = Rainbomizer::Common::GetVehicleHashes ();
                    return GetRandomElement (indices);
                }

            return CStreaming::GetModelByIndex (randomLoaded)->m_nHash;
        }
    }

    /*******************************************************/
    static bool
    RandomizeScriptVehicle (uint32_t &hash, Vector3_native *coords,
                            float heading, bool isNetwork, bool thisScriptCheck)
    {
        auto     thread       = scrThread::GetActiveThread ();
        uint32_t originalHash = hash;
        Vector3  pos          = {coords->x, coords->y, coords->z};

        if (!thread || !thread->IsYscScript ())
            return true;

        // Used for async loading of script vehicles. It's to ensure that the
        // mod doesn't try to change the model it was supposed to load.
        // Additional fail-safes are ideal, but aren't important
        static std::unordered_map<uint32_t, uint32_t> mThreadWaits;
        if (mThreadWaits.count (thread->m_Context.m_nScriptHash))
            {
                hash = mThreadWaits[thread->m_Context.m_nScriptHash];
                mThreadWaits.erase (thread->m_Context.m_nScriptHash);
            }
        else
            {
                hash = GetRandomHashForVehicle (hash, pos);
                if (Config ().LogSpawnedVehicles)
                    Rainbomizer::Logger::LogMessage (
                        "{%s:%d}: Spawning %x (%s) instead of %x (%s) at %.2f "
                        "%.2f "
                        "%.2f",
                        scrThread::GetActiveThread ()->m_szScriptName,
                        scrThread::GetActiveThread ()->m_Context.m_nIp, hash,
                        CStreaming::GetModelByHash<CVehicleModelInfo> (hash)
                            ->GetGameName (),
                        originalHash,
                        CStreaming::GetModelByHash<CVehicleModelInfo> (
                            originalHash)
                            ->GetGameName (),
                        coords->x, coords->y, coords->z);
            }

        uint32_t index = CStreaming::GetModelIndex (hash);

        // Wait for the model to launch, inhibit the hooked function call, and
        // set the script state to waiting. The game will keep executing this
        // native until the state is set back to Running
        if (!CStreaming::HasModelLoaded (index))
            {
                REQUEST_MODEL (hash);
                thread->m_Context.m_nState = eScriptState::WAITING;
                mThreadWaits[thread->m_Context.m_nScriptHash] = hash;
                return false;
            }

        coords->x = pos.x;
        coords->y = pos.y;
        coords->z = pos.z;

        thread->m_Context.m_nState = eScriptState::RUNNING;
        return true;
    }

    /*******************************************************/
    static bool
    IsVehDriveableHook (void *p1, bool p2, bool p3, bool p4)
    {
        return true;

        // return CVehicle__IsVehDriveable4137 (p1, p2, p3, p4);
    }

    /*******************************************************/
    static void
    InitialiseRandomVehiclesHook ()
    {
        RegisterHook ("? 8d ? ? ? ? ? ? 8a c1 ? 8a d1 e8 ? ? ? ? 84 c0 74 ?",
                      13, CVehicle__IsVehDriveable4137, IsVehDriveableHook);

        ReplaceJmpHook__fastcall<0x7c1c0, uint32_t, uint32_t, Vector3_native *,
                                 float, bool, bool> (
            hook::get_pattern ("8b ec ? 83 ec 50 f3 0f 10 02 f3 0f 10 4a 08 ",
                               -17),
            RandomizeScriptVehicle)
            .Activate ();
    }

public:
    /*******************************************************/
    ScriptVehicleRandomizer ()
    {
        if (!ConfigManager::ReadConfig (
                "ScriptVehicleRandomizer",
                std::pair ("LogSpawnedVehicles",
                           &Config ().LogSpawnedVehicles)))
            return;

        InitialiseAllComponents ();
        InitialiseRandomVehiclesHook ();
        InitialiseDLCDespawnFix ();
    }
};

ScriptVehicleRandomizer _scr;
