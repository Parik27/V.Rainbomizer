#pragma once

#include <cstdint>
#include "rage.hh"

enum class eDispatchId : uint32_t
{
    CPoliceAutomobileDispatch    = 1,
    CWantedHelicopterDispatch    = 2,
    CFireDepartmentDispatch      = 3,
    CSwatAutomobileDispatch      = 4,
    CAmbulanceDepartmentDispatch = 5,
    CPoliceRidersDispatch        = 6,
    CPoliceVehicleRequest        = 7,
    CPoliceRoadBlockDispatch     = 8,
    CGangDispatch                = 11,
    CSwatHelicopterDispatch      = 12,
    CPoliceBoatDispatch          = 13,
    CArmyVehicleDispatch         = 14
};

class CDispatchService
{
public:
    void **          vft;
    enum eDispatchId eId;
    uint32_t         field_0xc;
    atArray<>        field_0x10;
    uint32_t         field_0x1c;
    bool             bEnabled;
    bool             bResourceCreationEnabled;
    uint8_t          field_0x22;
    uint8_t          field_0x23;
    float            field_0x24;
    uint32_t         field_0x28;
    uint32_t         field_0x2c;
    uint32_t         field_0x30;
    uint8_t          field_0x34[8];
    uint16_t         field_0x3c;
    uint8_t          field_0x3e[82];
    uint32_t         field_0x90;
    uint32_t         field_0x94;
    uint8_t *        field_0x98;
    uint32_t         field_0xa0;
    uint32_t         field_0xa4;
};
