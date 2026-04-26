#pragma once

#include "memory/GameAddress.hh"
#include <cstdint>

struct CModelIndices
{
    inline static GameVariable<CModelIndices*, 100140> sm_Head{};

    static void InitialisePatterns ();

    GAME_ADDR_WRAPPER
    static CModelIndices *
    GetHead ()
    {
        return sm_Head;
    }

    uint32_t       nHash;
    uint32_t       nIndex;
    CModelIndices *pNext;
};
