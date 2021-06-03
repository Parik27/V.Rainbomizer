#pragma once

#include "mission/missions_Funcs.hh"
#include "mission/missions_Globals.hh"
#include "scrThread.hh"
#include <vector>

class MissionRandomizer_Commands
{
    using YF = YscFunctions;

    enum class eDoorState : uint32_t
    {
        UNLOCKED = 0,
        LOCKED,
        DOORSTATE_FORCE_LOCKED_UNTIL_OUT_OF_AREA,
        DOORSTATE_FORCE_UNLOCKED_THIS_FRAME,
        DOORSTATE_FORCE_LOCKED_THIS_FRAME,
        DOORSTATE_FORCE_OPEN_THIS_FRAME,
        DOORSTATE_FORCE_CLOSED_THIS_FRAME,
    };

    struct ForcedDoorStateInfo
    {
        uint32_t   DoorHash;
        eDoorState State;
    };

    inline static std::vector<ForcedDoorStateInfo> sm_ForcedDoorStates{};

    static void
    ForceDoorState (uint32_t hash, eDoorState state)
    {
        sm_ForcedDoorStates.push_back ({hash, state});
    }

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
    static void     SetMoney (uint32_t money);

    bool ProcessCleanupMissionTrigger ();
    bool ProcessShopUnlocks ();

    static void CreateStingerRamp ();

public:
    static void AdjustMissionFlowCommands ();

    static void OnMissionStart (uint32_t origHash, uint32_t randHash);
    static void OnMissionEnd (bool pass, uint32_t origHash, uint32_t randHash);
    static void OnMissionTick ();

    void CleanupMissionTriggerer ();

    bool
    Process (scrProgram *program, scrThreadContext *ctx)
    {
        return ProcessCleanupMissionTrigger ();
    }
};
