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
#include <CTheScripts.hh>
#include "mission/missions_YscUtils.hh"
#include <common/ysc.hh>

#ifdef ENABLE_DEBUG_MENU
#include "debug/base.hh"
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

    struct PatternResult
    {
        uint32_t Hash   = 0;
        bool     NoWait = false;
    };

    /*******************************************************/
    static void
    InitialisePatterns ()
    {
        static bool mPatternsInitialised = false;

#ifdef ENABLE_DEBUG_MENU
        if (DebugInterfaceManager::GetAction ("Reload Patterns"))
            {
                mPatternsInitialised = false;
                mPatterns.clear ();
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
    static PatternResult
    GetRandomHashForVehicle (uint32_t hash, Vector3 &coords)
    {
        const int     MIN_FREE_SLOTS = 25;
        PatternResult result;

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
                            {
                                result.Hash   = (returnLoaded)
                                                    ? i.GetRandomLoaded (coords)
                                                    : i.GetRandom (coords);
                                result.NoWait = i.GetFlags ().NoWait;
                                return result;
                            }
                    }
            }

        // Return a random vehicle
        {
            uint32_t numRandomLoaded = 0;
            uint32_t randomLoaded
                = VehicleRandomizerHelper::GetRandomLoadedVehIndex (
                    &numRandomLoaded);
            if (!returnLoaded && numRandomLoaded < 30)
                {
                    const auto &indices
                        = Rainbomizer::Common::GetVehicleHashes ();
                    result.Hash = GetRandomElement (indices);
                    return result;
                }

            result.Hash = CStreaming::GetModelByIndex (randomLoaded)->m_nHash;
            return result;
        }
    }

    /*******************************************************/
    static bool
    RandomizeScriptVehicle (uint32_t &hash, Vector3_native *coords,
                            float heading, bool isNetwork, bool thisScriptCheck)
    {
        auto          thread       = scrThread::GetActiveThread ();
        uint32_t      originalHash = hash;
        Vector3       pos          = {coords->x, coords->y, coords->z};
        PatternResult patternResult;

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
                patternResult = GetRandomHashForVehicle (hash, pos);
                hash          = patternResult.Hash;

                if (Config ().LogSpawnedVehicles)
                    Rainbomizer::Logger::LogMessage (
                        "{%s:%d}: Spawning %x (%s) instead of %x (%s) at %.2f "
                        "%.2f "
                        "%.2f %s",
                        scrThread::GetActiveThread ()->m_szScriptName,
                        scrThread::GetActiveThread ()->m_Context.m_nIp, hash,
                        CStreaming::GetModelByHash<CVehicleModelInfo> (hash)
                            ->GetGameName (),
                        originalHash,
                        CStreaming::GetModelByHash<CVehicleModelInfo> (
                            originalHash)
                            ->GetGameName (),
                        coords->x, coords->y, coords->z,
                        patternResult.NoWait ? "without waiting" : "");
            }

        uint32_t index = CStreaming::GetModelIndex (hash);

        // Wait for the model to launch, inhibit the hooked function call, and
        // set the script state to waiting. The game will keep executing this
        // native until the state is set back to Running
        if (!CStreaming::HasModelLoaded (index))
            {
                if (!patternResult.NoWait)
                    {
                        REQUEST_MODEL (hash);
                        thread->m_Context.m_nState = eScriptState::WAITING;
                        mThreadWaits[thread->m_Context.m_nScriptHash] = hash;
                        return false;
                    }
                else
                    {
                        REQUEST_MODEL (hash);
                        CStreaming::LoadAllObjects (false);

                        if (!CStreaming::HasModelLoaded (index))
                            hash = originalHash;
                    }
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

    /* This function makes it so the force applied to the vehicle at the end of
     * finalec2 is high enough so that it doesn't get stuck */
    /*******************************************************/
    static void
    FixFinaleC2Physics (scrThread::Info *info)
    {
        if (scrThread::CheckActiveThread ("finalec2"_joaat))
            info->GetArg<float> (4) = 75.0f;
    }

    /*******************************************************/
    static bool
    FixMichael2MissionDisruption (YscUtilsOps &ops)
    {
        if (!ops.IsAnyOf ("mission_triggerer_d"_joaat))
            return false;

        ops.Init ("6e 6e 6e 6e 6e 2c ? ? ? 20 56 ? ? 6f 2e 00 01 6e 2e 00 01");
        ops.Write (10, YscOpCode::J);

        return true;
    }

    /*******************************************************/
    static bool
    FixPilotSchoolHeliChallenge (YscUtilsOps &ops)
    {
        if (!ops.IsAnyOf ("pilot_school"_joaat))
            return false;

        ops.Init ("2c ? ? ? 2c ? ? ? 78 18 1f 2a 56");
        ops.NOP (/*offset=*/12, 3);

        return true;
    }

    /*******************************************************/
    static void
    RemoveTriathlonFailState (scrThread::Info *info)
    {
        if (scrThread::CheckActiveThread ("triathlonsp"_joaat))
            {
                if (info->GetArg (1) == "tribike"_joaat)
                    info->GetReturn () = true;
            }
    }

    /*******************************************************/
    static bool
    Fanatic3LeaveVehicleFix (YscUtilsOps &ops)
    {
        if (!ops.IsAnyOf ("fanatic3"_joaat))
            return false;

        static constexpr uint8_t return_1[] = {
            0x6f,          // PUSH_CONST_1
            0x2e, 0x1, 0x1 // LEAVE 0x1, 0x1
        };

        ops.Init ("2d 01 03 00 ? 38 ? 28 54 9c 77 43 08 ");
        ops.WriteBytes (/*Offset=*/5, return_1);

        return true;
    }

public:
    /*******************************************************/
    ScriptVehicleRandomizer ()
    {
#define HOOK(native, func) NativeCallbackMgr::Add<native##_joaat, func, true> ()
#define HOOK_A(native, func)                                                   \
    NativeCallbackMgr::Add<native##_joaat, func, false> ()

        if (!ConfigManager::ReadConfig (
                "ScriptVehicleRandomizer",
                std::pair ("LogSpawnedVehicles",
                           &Config ().LogSpawnedVehicles)))
            return;

        HOOK ("APPLY_FORCE_TO_ENTITY", FixFinaleC2Physics);
        HOOK_A ("IS_VEHICLE_MODEL", RemoveTriathlonFailState);
        HOOK_A ("IS_PED_IN_MODEL", RemoveTriathlonFailState);

        InitialiseAllComponents ();
        InitialiseRandomVehiclesHook ();
        VehicleRandomizerHelper::InitialiseDLCDespawnFix ();

        YscCodeEdits::Add ("Fix michael2 Mission disruption",
                           FixMichael2MissionDisruption);

        YscCodeEdits::Add ("Pilot School Helicopter Challenge Fix",
                           FixPilotSchoolHeliChallenge);

        YscCodeEdits::Add ("Fanatic3 Leave Vehicle Fix",
                           Fanatic3LeaveVehicleFix);

#undef HOOK
    }
};

ScriptVehicleRandomizer _scr;
