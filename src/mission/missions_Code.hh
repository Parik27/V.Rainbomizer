#pragma once

#include "common/logger.hh"
#include "missions_YscUtils.hh"

#include "missions_Data.hh"

#include "scrThread.hh"
#include "Utils.hh"
#include "rage.hh"

class MissionRandomizer_CodeFixes
{
    /*******************************************************/
    static bool ApplyStatWatcherFix (YscUtilsOps &utils);
    static bool ApplyCreditsFix_FinaleAB (YscUtilsOps &utils);
    static bool ApplyCreditsFix_FinaleC2 (YscUtilsOps &utils);
    static bool ApplyMissionPassFix (YscUtilsOps &utils);
    static bool ApplyTriggererWaitFix (YscUtilsOps &utils);
    static bool ApplyPrepNoRepeatFix (YscUtilsOps &utils);
    static bool ApplyQuickSkipsPatch (YscUtilsOps &utils);
    static bool ApplySolomonCamFix (YscUtilsOps &utils);
    static bool ApplyBuildingStatePatch (YscUtilsOps &utils);

public:
    void Initialise ();

    MissionRandomizer_CodeFixes () { Initialise (); }
};
