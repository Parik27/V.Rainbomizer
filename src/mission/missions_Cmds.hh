#pragma once

#include "mission/missions_Funcs.hh"
#include "mission/missions_Globals.hh"
#include "scrThread.hh"

class MissionRandomizer_Commands
{
    using YF = YscFunctions;

    bool bMissionTriggererCleanupRequested = false;
    
    static void MakeAllPlayersAvailable ();
    static void UnlockWeapon (ePlayerIndex player, uint32_t hash,
                              bool unlock = true, bool p4 = true);
    static void UnlockWeapon (uint32_t hash, bool unlock = true);
    static void AddContact (eCharacter dst, eCharacter contact);
    static void SetBuildingState (uint32_t building, uint32_t state);
    static bool ShouldUnlockSafehouse (uint32_t safehouse, uint32_t hash);
    static void UnlockSafehousesForMission (uint32_t hash);
    static void NopFlowCommands (unsigned int start, unsigned int end);

    static uint32_t GetMoney ();
    static void SetMoney (uint32_t money);

    bool ProcessCleanupMissionTrigger ();
    bool ProcessShopUnlocks ();

    static void CreateStingerRamp ();

public:

    static void AdjustMissionFlowCommands ();

    static void OnMissionStart (uint32_t origHash, uint32_t randHash);
    static void OnMissionEnd (bool pass, uint32_t origHash, uint32_t randHash);

    void CleanupMissionTriggerer ();

    bool
    Process (scrProgram *program, scrThreadContext *ctx)
    {
        return ProcessCleanupMissionTrigger ();
    }
};
