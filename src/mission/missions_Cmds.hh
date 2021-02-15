#pragma once

#include "mission/missions_Funcs.hh"
#include "mission/missions_Globals.hh"
#include "missions.hh"

class MissionRandomizer_Commands
{
    using MR = MissionRandomizer_Components;
    using YF = YscFunctions;

    /*******************************************************/
    static void
    MakeAllPlayersAvailable ()
    {
        *MR::sm_Globals.FLAG_PLAYER_PED_INTRODUCED_M = true;
        *MR::sm_Globals.FLAG_PLAYER_PED_INTRODUCED_T = true;
        *MR::sm_Globals.FLAG_PLAYER_PED_INTRODUCED_F = true;

        *MR::sm_Globals.SP0_AVAILABLE = true;
        *MR::sm_Globals.SP1_AVAILABLE = true;
        *MR::sm_Globals.SP2_AVAILABLE = true;
    }

    /*******************************************************/
    static void
    UnlockWeapon (ePlayerIndex player, uint32_t hash, bool unlock = true,
                  bool p4 = true)
    {
        YscFunctions::UnlockWeapon (player, hash, unlock, p4);
    }

    /*******************************************************/
    static void
    UnlockWeapon (uint32_t hash, bool unlock = true)
    {
        UnlockWeapon (ePlayerIndex::PLAYER_FRANKLIN, hash, unlock);
        UnlockWeapon (ePlayerIndex::PLAYER_MICHAEL, hash, unlock);
        UnlockWeapon (ePlayerIndex::PLAYER_TREVOR, hash, unlock);
    }

    /*******************************************************/
    static void
    AddContact (eCharacter dst, eCharacter contact)
    {
        YscFunctions::AddContact (contact, dst, 0);
    }

    /*******************************************************/
    static void
    SetBuildingState (uint32_t building, uint32_t state)
    {
        YscFunctions::SetBuildingState (building, state, 0, 1, 1);
    }

    /*******************************************************/
    static bool
    ShouldUnlockSafehouse (uint32_t safehouse, uint32_t hash)
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
    static void
    UnlockSafehousesForMission (uint32_t hash)
    {
        for (int i = 0; i <= 6; i++)
            {
                if (ShouldUnlockSafehouse (i, hash))
                    YF::SetSafehouseUnlocked (i, true);
            }
    }

public:
    /*******************************************************/
    static void
    OnMissionStart (uint32_t origHash, uint32_t randHash)
    {
        MakeAllPlayersAvailable ();
        UnlockSafehousesForMission (randHash);
        switch (randHash)
            {
            case "trevor2"_joaat:
                UnlockWeapon ("weapon_sniperrifle"_joaat);
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
                YF::SetMfControlFlag (FLAG_MOVIE_STUDIO_OPEN_FRAN, true);
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

            case "michael4"_joaat: "SET_CLOCK_TIME"_n(21, 00, 00); break;

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
                YF::SetMfControlFlag (FLAG_MOVIE_STUDIO_OPEN, true);
                YF::SetMfControlFlag (FLAG_MOVIE_STUDIO_OPEN_FRAN, true);
                break;

            case "carsteal1"_joaat:
            case "carsteal3"_joaat:
                YF::SetDoorState (79, 0);
                YF::SetDoorState (80, 0);
                break;

            case "finalec1"_joaat:
                YF::SetDoorState(70, 0);
                YF::SetDoorState(71, 0);
                YF::SetDoorState(72, 0);
                break;
            }
    }
};
