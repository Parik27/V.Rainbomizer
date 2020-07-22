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

CEntity *(*CPools__GetAtEntity) (int);

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
                
                Rainbomizer::Logger::LogMessage("%s => %d vehicles", line, pattern.GetNumVehicles());
                
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

ScriptVehicleRandomizer _scr;
