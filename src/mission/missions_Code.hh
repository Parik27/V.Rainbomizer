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
    static void PrintStatus (YscUtilsOps &utils, std::string_view fixName);

    /*******************************************************/
    static void ApplyStatWatcherFix (YscUtilsOps &utils);
    static void ApplyCreditsFix_FinaleAB (YscUtilsOps &utils);
    static void ApplyCreditsFix_FinaleC2 (YscUtilsOps &utils);
    static void ApplyMissionPassFix (YscUtilsOps &utils);

    /*******************************************************/
    template <auto &scrProgram_InitNativeTablese188>
    static bool
    ApplyCodeFixes (scrProgram *program)
    {
        YscUtilsOps utils (program);
        bool        ret = scrProgram_InitNativeTablese188 (program);

        ApplyStatWatcherFix (utils);
        ApplyStatWatcherFix (utils);
        ApplyCreditsFix_FinaleAB (utils);
        ApplyCreditsFix_FinaleC2 (utils);
        ApplyMissionPassFix (utils);

        return ret;
    }

public:
    MissionRandomizer_CodeFixes ()
    {
        REGISTER_HOOK ("8b cb e8 ? ? ? ? 8b 43 70 ? 03 c4 a9 00 c0 ff ff", 2,
                       ApplyCodeFixes, bool, scrProgram *);
    }
};
