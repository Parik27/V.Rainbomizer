#include <cstdint>
#include "rage.hh"

struct phCollider
{
    uint32_t       nType;
    uint32_t       field_0xc;
    struct phInst *pInstance;
    void *         field_0x18;
    rage::Vec3f    field_0x20;
    uint32_t       fInvMass;
    uint32_t       field_0x30;
    uint32_t       field_0x34;
    uint32_t       field_0x38;
    uint32_t       field_0x3c;
    rage::Vec3V    field_0x40;
    rage::Vec3V    field_0x50;
    rage::Vec3V    field_0x60;
    rage::Vec4V    vecVelocity;
    rage::Vec4V    vecAngVelocity;
    rage::Vec3V    field_0x90;
    rage::Vec3V    field_0xa0;
    rage::Vec3V    field_0xb0;
    rage::Vec3V    field_0xc0;
    uint32_t       field_0xd0;
    float          fMaxSpeed;
    float          fMaxAngSpeed;
    float          field_0xdc;
    rage::Vec3V    field_0xe0;
    rage::Vec3V    field_0xf0;
    uint8_t        field_0x100[32];
    rage::Vec3f    field_0x120;
    float          fMass;
    rage::Mat34V   field_0x130;
    rage::Mat34V   field_0x170;
    uint8_t        field_0x1b0[16];
    uint8_t        field_0x1c0[16];
    uint8_t        field_0x1d0[64];
    uint8_t        field_0x210[16];
    uint8_t        field_0x220[16];
    uint8_t        field_0x230[16];
    uint8_t        field_0x240[16];
    float          fRadius;
    uint8_t        field_0x254[12];
    uint8_t        field_0x260;
    uint8_t        field_0x261[15];

    virtual void Destructor (){};
};
