#include "missions_Cmds.hh"

#include "common/logger.hh"
#include "mission/missions_Funcs.hh"
#include "mission/missions_Globals.hh"
#include "missions.hh"
#include "rage.hh"

using MR = MissionRandomizer_Components;

/*******************************************************/
void
MissionRandomizer_Commands::MakeAllPlayersAvailable ()
{
    MR::sm_Globals.GetMfFlag (FLAG_PLAYER_PED_INTRODUCED_M) = true;
    MR::sm_Globals.GetMfFlag (FLAG_PLAYER_PED_INTRODUCED_T) = true;
    MR::sm_Globals.GetMfFlag (FLAG_PLAYER_PED_INTRODUCED_F) = true;

    *MR::sm_Globals.SP0_AVAILABLE = true;
    *MR::sm_Globals.SP1_AVAILABLE = true;
    *MR::sm_Globals.SP2_AVAILABLE = true;
}

/*******************************************************/
void
MissionRandomizer_Commands::UnlockWeapon (ePlayerIndex player, uint32_t hash,
                                          bool unlock, bool p4)
{
    YscFunctions::UnlockWeapon (player, hash, unlock, p4);
}

/*******************************************************/
void
MissionRandomizer_Commands::UnlockWeapon (uint32_t hash, bool unlock)
{
    UnlockWeapon (ePlayerIndex::PLAYER_FRANKLIN, hash, unlock);
    UnlockWeapon (ePlayerIndex::PLAYER_MICHAEL, hash, unlock);
    UnlockWeapon (ePlayerIndex::PLAYER_TREVOR, hash, unlock);
}

/*******************************************************/
void
MissionRandomizer_Commands::AddContact (eCharacter dst, eCharacter contact)
{
    YscFunctions::AddContact (contact, dst, 0);
}

/*******************************************************/
void
MissionRandomizer_Commands::SetBuildingState (uint32_t building, uint32_t state)
{
    YscFunctions::SetBuildingState (building, state, 0, 1, 1);
}

/*******************************************************/
bool
MissionRandomizer_Commands::ShouldUnlockSafehouse (uint32_t safehouse,
                                                   uint32_t hash)
{
    switch (safehouse)
        {
        case 0:
            switch (hash)
                {
                case "family1"_joaat:
                case "family2"_joaat:
                case "family3"_joaat:
                case "family4"_joaat:
                case "family5"_joaat:
                case "family6"_joaat:
                case "lester1"_joaat:
                case "michael1"_joaat:
                case "michael4"_joaat: return true;
                }
            break;

        case 3:
            switch (hash)
                {
                case "docks_heista"_joaat:
                case "docks_heistb"_joaat:
                case "docks_setup"_joaat: return true;
                }
            break;

        case 6:
            switch (hash)
                {
                case "agency_heist3b"_joaat:
                case "franklin2"_joaat: return true;
                }
            break;
        }

    return false;
}

/*******************************************************/
void
MissionRandomizer_Commands::UnlockSafehousesForMission (uint32_t hash)
{
    for (int i = 0; i <= 6; i++)
        {
            if (ShouldUnlockSafehouse (i, hash))
                YF::SetSafehouseUnlocked (i, true);
        }
}

/*******************************************************/
void
MissionRandomizer_Commands::NopFlowCommands (unsigned int start,
                                             unsigned int end)
{
    for (unsigned int i = start; i <= end; ++i)
        {
            // Just set it to an invalid flow command, it'll skip it.
            if (auto cmds = MR::sm_Globals.g_MissionFlowCommands.Get ())
                cmds->Data[i].CommandHash = "123robot"_joaat;
        }
}

/*******************************************************/
bool
MissionRandomizer_Commands::ProcessCleanupMissionTrigger ()
{
    if (!bMissionTriggererCleanupRequested)
        return true;

    if (!YF::ResetTrigEntityProofs.CanCall (true))
        return false;

    if (MR::sm_Globals.g_ForceWalking)
        YF::ResetTrigEntityProofs ();

    "SET_PLAYER_CONTROL"_n("PLAYER_ID"_n(), true, 0);
    YF::CleanupTrigEntities ();

    bMissionTriggererCleanupRequested = false;
    return true;
}

/*******************************************************/
void
MissionRandomizer_Commands::AdjustMissionFlowCommands ()
{
    // Prologue Freeze
    NopFlowCommands (1557, 1558);

    // Heist Control Scripts
#if (0)
    NopFlowCommands (7, 7);
    NopFlowCommands (357, 357);
    NopFlowCommands (1011, 1011);
    NopFlowCommands (1199, 1199);
    NopFlowCommands (1566, 1566);
#endif

    // Agency Boards
    NopFlowCommands (34, 34);
    NopFlowCommands (36, 49);

    // Docks Board
    NopFlowCommands (363, 363);
    NopFlowCommands (369, 378);

    // Finale Heist Board (+ finale_heist_intro2)
    NopFlowCommands (1017, 1033);

    // Jewel Heist Board
    NopFlowCommands (1206, 1220);

    // Rural Bank Heist Board
    NopFlowCommands (1577, 1579);
    NopFlowCommands (1585, 1586);
}

/*******************************************************/
void
MissionRandomizer_Commands::OnMissionStart (uint32_t origHash,
                                            uint32_t randHash)
{
    MR::sm_Cmds.CleanupMissionTriggerer ();
    MakeAllPlayersAvailable ();
    UnlockSafehousesForMission (randHash);
    switch (randHash)
        {
        case "trevor2"_joaat:
            UnlockWeapon ("weapon_sniperrifle"_joaat);
            YF::SetShopState (29, true, 0);
            break;

        case "lamar1"_joaat:
            UnlockWeapon ("weapon_sawnoffshotgun"_joaat);
            UnlockWeapon ("weapon_pumpshotgun"_joaat);
            YF::SetDoorState (101, 0);
            YF::SetDoorState (102, 0);
            YF::SetDoorState (103, 0);
            YF::SetDoorState (104, 0);
            break;

        case "docks_setup"_joaat:
            AddContact (eCharacter::Trevor, eCharacter::Ron);
            [[fallthrough]];

        case "docks_heista"_joaat:
            SetBuildingState (12, 0);
            SetBuildingState (13, 0);
            break;

        case "armenian1"_joaat:
            MR::sm_Globals.GetMfFlag (FLAG_MOVIE_STUDIO_OPEN_FRAN) = true;            
            [[fallthrough]];
        case "armenian2"_joaat: SetBuildingState (179, 0); [[fallthrough]];
        case "armenian3"_joaat:
            SetBuildingState (174, 0);
            SetBuildingState (178, 0);
            SetBuildingState (175, 0);
            SetBuildingState (176, 0);
            SetBuildingState (177, 0);
            SetBuildingState (173, 0);
            break;

        case "michael4"_joaat:
            "SET_CLOCK_TIME"_n(21, 00, 00);
            YF::SetShopState (18, true, 0);
            break;

        case "agency_heist1"_joaat:
        case "agency_heist2"_joaat:
        case "agency_heist3a"_joaat:
        case "agency_heist3b"_joaat:
        case "jewelry_heist"_joaat:
        case "jewelry_prep2a"_joaat:
        case "jewelry_setup1"_joaat:

            SetBuildingState (70, 1);
            SetBuildingState (71, 1);
            SetBuildingState (73, 1);
            SetBuildingState (72, 0);
            SetBuildingState (74, 0);

            YF::SetDoorState (62, 0);
            YF::SetDoorState (63, 0);

            break;

        case "chinese1"_joaat:
        case "rural_bank_heist"_joaat:
        case "rural_bank_setup"_joaat:

            YF::SetDoorState (67, 0);
            YF::SetDoorState (68, 0);
            YF::SetDoorState (69, 0);
            break;

        case "solomon1"_joaat:
        case "solomon2"_joaat:
        case "solomon3"_joaat:
            MR::sm_Globals.GetMfFlag (FLAG_MOVIE_STUDIO_OPEN)      = true;
            MR::sm_Globals.GetMfFlag (FLAG_MOVIE_STUDIO_OPEN_FRAN) = true;
            break;

        case "carsteal1"_joaat:
        case "carsteal3"_joaat:
            YF::SetDoorState (79, 0);
            YF::SetDoorState (80, 0);
            break;

        case "finalec1"_joaat:
            YF::SetDoorState (70, 0);
            YF::SetDoorState (71, 0);
            YF::SetDoorState (72, 0);
            break;

        case "fbi4_prep4"_joaat:
            YF::SetShopState (21, true, 0);
            SetMoney(std::max(150u, GetMoney()));
            break;

        case "fbi4_prep5"_joaat:
            YF::SetShopState (28, true, 0);
            SetMoney(std::max(315u, GetMoney()));
            break;

        case "finale_heist_prepc"_joaat:
            SetMoney (std::max (11000u, GetMoney ()));
            break;

        case "family3"_joaat: SetBuildingState (2, 0); break;
        }
}

void
MissionRandomizer_Commands::CleanupMissionTriggerer ()
{
    if (!YF::CleanupTrigEntities.ThreadExists ()
        || !YF::CleanupTrigEntities.CanCall (false))
        {
            bMissionTriggererCleanupRequested = false;
            return;
        }
    bMissionTriggererCleanupRequested = true;
}

uint32_t
MissionRandomizer_Commands::GetMoney ()
{
    uint32_t currentPed = MR::sm_PlayerSwitcher.GetDestPlayer();
    uint32_t statName   = 0;
    uint32_t money      = 0;

    switch (currentPed)
        {
        case 0: statName = "sp0_total_cash"_joaat; break;
        case 1: statName = "sp1_total_cash"_joaat; break;
        case 2: statName = "sp2_total_cash"_joaat; break;
        }

    "STAT_GET_INT"_n(statName, &money, -1);
    Rainbomizer::Logger::LogMessage ("GetMoney: %d (%d)", money, currentPed);
    return money;
}

void
MissionRandomizer_Commands::SetMoney (uint32_t money)
{
    uint32_t currentPed = MR::sm_PlayerSwitcher.GetDestPlayer ();
    uint32_t statName   = 0;

    switch (currentPed)
        {
        case 0: statName = "sp0_total_cash"_joaat; break;
        case 1: statName = "sp1_total_cash"_joaat; break;
        case 2: statName = "sp2_total_cash"_joaat; break;
        }

    Rainbomizer::Logger::LogMessage ("SetMoney: %d (%d)", money, currentPed);
    "STAT_SET_INT"_n(statName, money, 1);
}

void
MissionRandomizer_Commands::OnMissionEnd (bool pass, uint32_t origHash,
                                          uint32_t randHash)
{
    MR::sm_Globals.GetMfFlag (FLAG_H_JEWEL_PRIME_BOARD)  = false;
    MR::sm_Globals.GetMfFlag (FLAG_H_AGENCY_PRIME_BOARD) = false;
    MR::sm_Globals.GetMfFlag (FLAG_H_DOCKS_PRIME_BOARD)  = false;
    MR::sm_Globals.GetMfFlag (FLAG_H_FINALE_PRIME_BOARD) = false;
}
