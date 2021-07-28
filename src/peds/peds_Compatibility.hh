#pragma once

#include "CModelInfo.hh"
#include <cstdint>
#include <map>

class CPed;

class PedRandomizer_Compatibility
{
    inline static std::map<CPed *, std::pair<uint32_t, uint32_t>>
        sm_PedModelFixupMap;

    inline static CPedModelInfo *sm_CurrentRandomizingPed = nullptr;

    static bool IsPlayerModel (uint32_t model);

    /*******************************************************/
    static void
    AddRandomizedPed (CPed *ped, uint32_t from, uint32_t to)
    {
        sm_PedModelFixupMap[ped] = std::make_pair (from, to);
    }

    /*******************************************************/
    static void
    SetRandomizingPed (CPedModelInfo *model)
    {
        sm_CurrentRandomizingPed = model;
    }

public:
    /*******************************************************/
    static CPedModelInfo *GetOriginalModel (CPed *ped);

    static CPedModelInfo *
    GetCurrentPedRandomizing ()
    {
        return sm_CurrentRandomizingPed;
    }

    friend class PedRandomizer;
    friend class PedRandomizer_AnimalFixes;
};
