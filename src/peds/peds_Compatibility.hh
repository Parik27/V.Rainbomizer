#pragma once

#include "CModelInfo.hh"
#include <cstdint>

class CPed;

class PedRandomizerCompatibility
{
    inline static std::map<CPed *, std::pair<uint32_t, uint32_t>>
        sm_PedModelFixupMap;

    inline static std::map<uint32_t, uint32_t> sm_ModelMap;

    static bool IsPlayerModel (uint32_t model);

    /*******************************************************/
    static void AddRandomizedPed (CPed *ped, uint32_t from, uint32_t to);

public:
    /*******************************************************/
    static CPedModelInfo *GetOriginalModel (CPed *ped);

    /*******************************************************/
    static uint32_t
    GetLastRandomModelForHash (uint32_t hash)
    {
        if (sm_ModelMap.count (hash))
            return sm_ModelMap[hash];

        return hash;
    }

    friend class PedRandomizer;
};
