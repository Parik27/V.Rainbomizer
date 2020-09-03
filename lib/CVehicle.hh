#pragma once

#include <CEntity.hh>
#include <CARGB.hh>

class CCustomShaderEffectVehicle
{
public:
    void *   vft;
    uint32_t flags;
    // ...

    CARGB *GetColours ();
};

class CVehicle : public CEntity
{
};
