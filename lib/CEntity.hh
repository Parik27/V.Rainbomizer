#pragma once

#include <cstdio>
#include <cstddef>
#include <CModelInfo.hh>

class CEntity
{    
public:
    
    void* vft;
    uint8_t __pad00[24];
    
    CBaseModelInfo *m_pModelInfo;
};

static_assert (offsetof (CEntity, m_pModelInfo) == 32);
