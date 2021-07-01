#include "CStreaming.hh"
#include <cstdint>
#include <cstdio>

#include <common/common.hh>
#include <CStreaming.hh>

class PedRandomizer_Stats
{
public:
    static void
    AddPedSpawn (uint32_t ped)
    {
        static FILE *f
            = Rainbomizer::Common::GetRainbomizerFile ("PedStats.dat", "a");
        ped = CStreaming::GetModelHash (ped);
        fwrite (&ped, 4, 1, f);
        fflush (f);
    }
};
