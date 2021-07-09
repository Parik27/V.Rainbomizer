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

    const MissionInfo *OriginalMission   = nullptr;
    const MissionInfo *RandomizedMission = nullptr;

    ePlayerIndex nPlayerIndexBeforeMission = ePlayerIndex::PLAYER_UNKNOWN;
    ePlayerIndex nPlayerIndexOnMissionFail = ePlayerIndex::PLAYER_UNKNOWN;

    uint32_t nLastPassedMissionTime  = 0;
    uint32_t nPreviousCurrentMission = -1u;

    int32_t nMissionPtrsSema = 2;

    bool bCallMissionEndNextFrame = false;

    bool bMissionRepeating         = false;
    bool bGameInitialised          = false;
    bool bInitialCutsceneRemoved   = false;
    bool bMissionStartupFinished   = false;
    bool bTriggererCleanupFinished = false;

    void HandleCurrentMissionChanges ();
    void InitStatWatcherForRandomizedMission ();
    void FixMissionRepeatStructForRandomizedMission ();
    bool HandleCutscenesForRandomizedMission ();

    bool HandleHeistCrewRandomization (scrThreadContext *ctx);

    auto GenerateSwitcherContext (bool start);

    bool WasMissionPassed ();
    bool PreMissionStart ();
    bool OnMissionStart ();
    bool OnMissionEnd (bool passed);

    void LogPlayerPos (bool start);

    void SetFlowControlVariableForHeist (eHeistId id, bool approach);
    void SetHeistFlowControlVariables ();
    void SetHeistFlowControlVariablesForMission ();

public:
    bool ShouldSetVariables (scrThreadContext *ctx);
    void SetVariables (scrThreadContext *ctx);
    void ClearVariables (scrThreadContext *ctx);

    bool Process (scrProgram *program, scrThreadContext *ctx);

    void Reset ();
};
