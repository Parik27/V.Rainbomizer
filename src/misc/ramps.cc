#include "CMath.hh"
#include "CStreaming.hh"
#include "common/logger.hh"
#include "rage.hh"
#include "CTheScripts.hh"

#include <common/events.hh>
#include <common/config.hh>
#include <common/common.hh>

#include <stdio.h>
#include <string.h>

using namespace NativeLiterals;

class RampRandomizer
{
    struct RampInfo
    {
        uint32_t Thread;
        uint32_t Model;
        Vector3  Pos;
        Vector4  Rot;
        uint32_t ObjectHandle = -1;

        /*******************************************************/
        void
        Spawn ()
        {
            Rainbomizer::Logger::LogMessage (
                "Spawning ramp: %x %x %f %f %f %f %f %f", Thread, Model, Pos.x,
                Pos.y, Pos.z, Rot.x, Rot.y, Rot.z);

            uint32_t modelIdx = CStreaming::GetModelIndex (Model);

            if ("DOES_ENTITY_EXIST"_n(ObjectHandle))
                return;

            if (!CStreaming::HasModelLoaded (modelIdx))
                {
                    CStreaming::RequestModel (modelIdx, 0);
                    CStreaming::LoadAllObjects (false);
                }

            ObjectHandle
                = "CREATE_OBJECT_NO_OFFSET"_n(Model, Pos.x, Pos.y, Pos.z, false,
                                              false, false);

            "SET_ENTITY_QUATERNION"_n(ObjectHandle, Rot.x, Rot.y, Rot.z, Rot.w);
        }
    };

    static inline std::vector<RampInfo> sm_RampInfos;

    /*******************************************************/
    static void
    InitialiseRamps ()
    {
        if (sm_RampInfos.size ())
            return;

        FILE *f = Rainbomizer::Common::GetRainbomizerDataFile ("Ramps.txt");
        if (!f)
            return;

        char line[2048] = {0};
        while (fgets (line, 2048, f))
            {
                if (line[0] == '#')
                    continue;

                RampInfo info;
                line[strcspn (line, "\n")] = 0;

                char ThreadName[256] = {0};
                char ModelName[256]  = {0};

                if (sscanf (line, "%s %s %f %f %f %f %f %f %f", ThreadName,
                            ModelName, &info.Pos.x, &info.Pos.y, &info.Pos.z,
                            &info.Rot.x, &info.Rot.y, &info.Rot.z, &info.Rot.w)
                    != 9)
                    continue;

                info.Thread = rage::atStringHash (ThreadName);
                info.Model  = rage::atStringHash (ModelName);
                sm_RampInfos.push_back (info);
            }

        fclose (f);
    }

    /*******************************************************/
    static void
    HandleScriptStart (uint32_t hash)
    {
        InitialiseRamps ();
        for (auto &i : sm_RampInfos)
            {
                if (i.Thread == hash)
                    i.Spawn ();
            }
    }

public:
    /*******************************************************/
    RampRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("RampRandomizer"))
            return;

        Rainbomizer::Events ().OnScriptStart += HandleScriptStart;
    }
} _rampRando;
