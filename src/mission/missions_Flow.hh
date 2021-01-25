#pragma once

#include "scrThread.hh"

#include "missions_Globals.hh"
#include "missions_Data.hh"
#include <cstdint>

class MissionRandomizer_Flow
{
    uint32_t nOriginalMission;
    uint32_t nRandomizedMission;
    const MissionData* pOriginalMission;
    const MissionData* pRandomizedMission;
    
    uint32_t nPreviousCurrentMission = -1u;
    bool bGameInitialised = false;

    void HandleCurrentMissionChanges ();

    bool OnMissionStart ();
    bool OnMissionEnd (bool passed);
    
public:
    bool Process (scrProgram *program, scrThreadContext *ctx);

    void Reset ();
};
