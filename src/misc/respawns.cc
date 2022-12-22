#include "common/config.hh"
#include "CMath.hh"
#include "CPed.hh"
#include "Utils.hh"
#include "common/common.hh"
#include "common/logger.hh"
#include "mission/missions_YscUtils.hh"
#include "scrThread.hh"
#include <cstdint>
#include <cstdio>

#include <CTheScripts.hh>

#include <common/ysc.hh>
#include <common/config.hh>
#include <common/events.hh>
#include <stdio.h>
#include <vector>

using namespace NativeLiterals;

class RespawnRandomizer
{
    struct Offsets
    {
        uint32_t Coordinates;
        uint32_t Switch;
    } inline static m_Offsets[2];

    static auto &
    Config ()
    {
        struct
        {
            bool MiddleOfNowhere = true;
        } static m_Config;

        return m_Config;
    }

    inline static std::vector<Vector3> sm_CoordsList;
    inline static bool                 sm_Initialised = false;

    /*******************************************************/
    static bool
    ReadCoordsList ()
    {
        static bool sm_ReadCoords = false;

        if (sm_ReadCoords)
            return sm_CoordsList.size ();

        sm_ReadCoords = true;

        FILE *f
            = Rainbomizer::Common::GetRainbomizerDataFile ("RespawnCoords.txt");

        if (!f)
            return false;

        char line[512] = {0};
        while (fgets (line, 512, f))
            {
                constexpr Vector3 offset
                    = {5.32516471875, 3.58871494336, 0.13737890136};

                Vector3 pos;
                line[strcspn (line, "\n")] = 0;

                if (sscanf (line, "%f %f %f", &pos.x, &pos.y, &pos.z) != 3)
                    continue;

                pos.x += offset.x;
                pos.y += offset.y;
                pos.z += offset.z;
                sm_CoordsList.push_back (pos);
            }

        return sm_CoordsList.size ();
    }

    /*******************************************************/
    static bool
    InitialiseRespawnRandomizer (YscUtilsOps &ops)
    {
        if (!ops.IsAnyOf ("respawn_controller"_joaat))
            return false;

        if (Config ().MiddleOfNowhere && !ReadCoordsList ())
            return false;

        ops.Init ("29 10 9a df c3 29 75 6b ab c3 29 0c 02 0a 42");
        m_Offsets[0].Coordinates = ops.GetWorkingIp ();

#ifdef ENABLE_SET_GROUND_Z_COORDS
        if (Config ().MiddleOfNowhere)
            {
                ops.Init ("38 ? 40 ? 41 ? 6e 6f 5d ? ? ? 2b");
                ops.Write (6, YscOpCode::PUSH_CONST_1);
            }
#endif

        ops.Init ("43 ? ? 63 3a ? 42 ? 43 ? ? 63 3a ? 42 ? 38 ? 62 05");
        m_Offsets[0].Switch = ops.GetWorkingIp ();

        if (Config ().MiddleOfNowhere)
            {
                ops.Write (16, ops.OpCode (PUSH_CONST_0));
                ops.Write (17, ops.OpCode (NOP));
            }

        sm_Initialised = true;
        return true;
    }

    /*******************************************************/
    static void
    RunThreadHook (uint64_t *, uint64_t *, scrProgram *prog,
                   scrThreadContext *ctx)
    {
        if (ctx->m_nScriptHash != "respawn_controller"_joaat || !sm_Initialised)
            return;

        YscUtilsOps ops (prog);

        if (Config ().MiddleOfNowhere)
            {
                auto &coords = GetRandomElement (sm_CoordsList);

                ops.Init (m_Offsets[0].Coordinates);
                ops.Write (1, coords.x);  // x
                ops.Write (6, coords.y);  // y
                ops.Write (11, coords.z); // z
            }
        else
            {
                ops.Init (m_Offsets[0].Switch);
                ops.Write (16, ops.OpCode(PUSH_CONST_U8));
                ops.Write (17, RandomInt (6));
            }
    }

public:
    RespawnRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("RespawnRandomizer"))
            return;

        YscCodeEdits::Add ("Initialise Respawn Randomizer",
                           InitialiseRespawnRandomizer);

        Rainbomizer::Events ().OnRunThread += RunThreadHook;
    }
} respawn;
