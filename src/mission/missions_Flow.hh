#pragma once

#include "common/logger.hh"
#include "scrThread.hh"

#include "missions_Globals.hh"
#include "missions_Data.hh"
#include "missions_Order.hh"
#include <cstdint>

#include "common/common.hh"

class MissionRandomizer_Flow
{
    FILE *mStartCoordsFile
        = Rainbomizer::Common::GetRainbomizerFile ("startCoords.txt", "a");
    FILE *mEndCoordsFile
        = Rainbomizer::Common::GetRainbomizerFile ("endCoords.txt", "a");

    const MissionInfo* OriginalMission = nullptr;
    const MissionInfo* RandomizedMission = nullptr;

    ePlayerIndex nPlayerIndexBeforeMission = ePlayerIndex::PLAYER_UNKNOWN;
    uint32_t     nLastPassedMissionTime    = 0;
    uint32_t     nPreviousCurrentMission   = -1u;

    bool bCallMissionEndNextFrame = false;

    bool bScriptEnded      = false;
    bool bMissionRepeating = false;
    bool bGameInitialised  = false;

    void HandleCurrentMissionChanges ();
    void InitStatWatcherForRandomizedMission ();

    bool WasMissionPassed ();
    bool PreMissionStart ();
    bool OnMissionStart ();
    bool OnMissionEnd (bool passed);

    void LogPlayerPos (bool start);
    
public:
    bool Process (scrProgram *program, scrThreadContext *ctx);

    void Reset ();
};
