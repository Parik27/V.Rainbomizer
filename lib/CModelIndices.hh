#pragma once

#include <cstdint>

struct CModelIndices
{
    inline static CModelIndices **sm_Head = nullptr;

    static void InitialisePatterns ();
    static CModelIndices *
    GetHead ()
    {
        return *sm_Head;
    }

    uint32_t       nHash;
    uint32_t       nIndex;
    CModelIndices *pNext;
};
