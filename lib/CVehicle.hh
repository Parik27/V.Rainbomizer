#pragma once

#include "memory/GameAddress.hh"
#include <CEntity.hh>
#include <CARGB.hh>

class CCustomShaderEffectVehicle
{
public:
    void *   vft;
    uint32_t flags;
    // ...

    CARGB *GetColours ()
    {
        return GameOffset<100152>::Get<CARGB*>(this);
    }
};

class CVehicle : public CEntity
{
};
