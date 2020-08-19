#pragma once

#include <cstdint>
#include "rage.hh"

struct tcVarInfo
{
    uint8_t field_0x0[8];
    char *  sAttributeName;
    uint8_t field_0x10[16];
};

static_assert (sizeof (tcVarInfo) == 0x20);

class tcConfig
{
private:
    inline static uint32_t *  ms_numVars;
    inline static tcVarInfo **ms_pVarInfos;

public:
    inline static uint32_t &
    GetNumVars ()
    {
        return *ms_numVars;
    }

    inline static tcVarInfo *&
    GetVarInfos ()
    {
        return *ms_pVarInfos;
    }

    inline static atArray<char[64]> *ms_cycleInfo;

    friend class tcManager;
};

struct tcCycleInfo
{
    struct
    {
        struct
        {
            rage::half Variables[432]; // these are half-precision floats
        } TimeSamples[13];
    } Regions[2];
    uint32_t Hash;
    uint8_t  field_0x57c4[28];
};

static_assert (sizeof (tcCycleInfo) == 22496);

// sizeof = 10340 (in v1)
class tcManager
{
public:
    uint8_t field_0x0[104]; // contains apModifiers, aModifierHashMap and stuff
    tcCycleInfo *pTimecycles;

    inline static tcManager *g_timeCycle;

    static void InitialisePatterns ();
};
