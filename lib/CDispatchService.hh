#pragma once

#include <cstdint>
#include "rage.hh"

enum class eDispatchId : uint32_t
{
    DT_Invalid = 0,
    DT_PoliceAutomobile,
    DT_PoliceHelicopter,
    DT_FireDepartment,
    DT_SwatAutomobile,
    DT_AmbulanceDepartment,
    DT_PoliceRiders,
    DT_PoliceVehicleRequest,
    DT_PoliceRoadBlock,
    DT_PoliceAutomobileWaitPulledOver,
    DT_PoliceAutomobileWaitCruising,
    DT_Gangs,
    DT_SwatHelicopter,
    DT_PoliceBoat,
    DT_ArmyVehicle,
    DT_BikerBackup
};

class CDispatchService
{
public:
    void **          vft;
    eDispatchId      eId;
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
