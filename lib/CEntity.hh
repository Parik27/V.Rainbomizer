#pragma once

#include <cstdio>
#include <cstddef>
#include <CModelInfo.hh>
#include <fwExtensibleBase.hh>

class CEntity
{
public:
    void *          vft;
    void *          m_pRefHolder;
    fwExtensionList m_List;
    CBaseModelInfo *m_pModelInfo;
    uint32_t        m_Type;
};

static_assert (offsetof (CEntity, m_pModelInfo) == 32);
