#pragma once

#include "CModelInfo.hh"
#include <cstdint>
#include <map>

class CPed;

class PedRandomizerCompatibility
{
    inline static std::map<CPed *, std::pair<uint32_t, uint32_t>>
        sm_PedModelFixupMap;

    static bool IsPlayerModel (uint32_t model);

    /*******************************************************/
    static void
    AddRandomizedPed (CPed *ped, uint32_t from, uint32_t to)
    {
        sm_PedModelFixupMap[ped] = std::make_pair (from, to);
    }

public:
    /*******************************************************/
    static CPedModelInfo *GetOriginalModel (CPed *ped);

    friend class PedRandomizer;
};
