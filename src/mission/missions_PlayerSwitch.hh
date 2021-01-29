#pragma once

#include "Patterns/Patterns.hh"
#include "common/logger.hh"
#include "mission/missions_Globals.hh"
#include "mission/missions_YscUtils.hh"
#include "mission/missions_Funcs.hh"
#include "rage.hh"
#include "scrThread.hh"

#include <CTheScripts.hh>
#include <CStreaming.hh>

#include <utility>
#include <cstdint>
#include <ctime>

#include "missions_Globals.hh"

using namespace NativeLiterals;

class MissionRandomizer_PlayerSwitcher
{
    enum
        {
            IDLE,
            TRANSITION_BEGIN,
            TRANSITION_ASCEND,
            PLAYER_SWITCH,
            TRANSITION_END,
            TRANSITION_DESCEND
        } m_nCurrentState
    = IDLE;

    rage::Vec3V  destPos;
    ePlayerIndex destPlayer;
    bool         bNoTransition = false;

public:
    /*******************************************************/
    void
    Reset ()
    {
        m_nCurrentState = IDLE;
    }

    /*******************************************************/
    void
    BeginSwitch (const rage::Vec3V &dstPos, ePlayerIndex dstPlayer,
                 bool noTransition)
    {
        if (m_nCurrentState != IDLE)
            return;

        Rainbomizer::Logger::LogMessage (
            "Beginning Player Switch: %f %f %f (%d) - %s", dstPos.x, dstPos.y,
            dstPos.z, dstPlayer, noTransition ? "true" : "false");

        destPos    = dstPos;
        destPlayer = dstPlayer;

        m_nCurrentState = TRANSITION_BEGIN;
        if (noTransition)
            m_nCurrentState = PLAYER_SWITCH;

        this->bNoTransition = noTransition;
    }

    /*******************************************************/
    uint32_t
    GetSetCurrentPlayerInstruction (YscUtils &program)
    {
        static uint32_t ip = 0;
        if (!ip)
            program.FindCodePattern (
                                     "2d 02 0b 00 ? 6e 52 ? ? 42 ? 2c ? ? ? "
                                     "2c ? ? ? 56 ? ? 6f 2e 02 01",
                [&] (hook::pattern_match m) {
                    ip = program.GetCodeOffset (m.get<uint8_t> ());
                });

        return ip;
    }

    /*******************************************************/
    bool
    SetCurrentPlayer (ePlayerIndex index)
    {
        if (!YscFunctions::SetCurrentPlayer (index, 1))
            return true;

        CStreaming::LoadAllObjects (false);

        bool success
            = *MissionRandomizer_GlobalsManager::PP_CURRENT_PED == int (index);

        if (!success)
            {
                static auto lastPrint = 0ll;
                if (time (NULL) - lastPrint > 2)
                    {
                        lastPrint = time (NULL);
                        Rainbomizer::Logger::LogMessage (
                            "Failed to set player model. current player: %x, "
                            "dst player: %x",
                            *MissionRandomizer_GlobalsManager::PP_CURRENT_PED,
                            destPlayer);
                    }
            }

        return success;
    }

    /*******************************************************/
    bool
    Process ()
    {
        switch (m_nCurrentState)
            {
            case IDLE: return true;

                case TRANSITION_BEGIN: {

                    if ("IS_PLAYER_SWITCH_IN_PROGRESS"_n())
                        break;
                    
                    "_SWITCH_OUT_PLAYER"_n("PLAYER_PED_ID"_n(), 0, 1);
                    m_nCurrentState = TRANSITION_ASCEND;

                    Rainbomizer::Logger::LogMessage (
                        "Player switched out: %d",
                        *MissionRandomizer_GlobalsManager::PP_CURRENT_PED);
                    break;
                }

                case TRANSITION_ASCEND: {
                    if ("_933BBEEB8C61B5F4"_n()) {
                        Rainbomizer::Logger::LogMessage (
                        "Player switcher ascent finished: %d",
                        *MissionRandomizer_GlobalsManager::PP_CURRENT_PED);
                        m_nCurrentState = PLAYER_SWITCH;
                    }
                    break;
                }

                case PLAYER_SWITCH: {
                    if (SetCurrentPlayer (destPlayer))
                        {
                            Rainbomizer::Logger::LogMessage (
                                "Setting current player successful: %d",
                                *MissionRandomizer_GlobalsManager::
                                    PP_CURRENT_PED);

                            "SET_PED_COORDS_KEEP_VEHICLE"_n("PLAYER_PED_ID"_n(),
                                                            destPos.x,
                                                            destPos.y,
                                                            destPos.z);
                            m_nCurrentState = TRANSITION_END;
                        }
                    break;
                };

                case TRANSITION_END: {
                    if (!bNoTransition)
                        {
                            "_SWITCH_IN_PLAYER"_n("PLAYER_PED_ID"_n());
                            Rainbomizer::Logger::LogMessage (
                                "Player switched in: %d",
                                *MissionRandomizer_GlobalsManager::
                                    PP_CURRENT_PED);

                            m_nCurrentState = TRANSITION_DESCEND;
                            break;
                        }
                    m_nCurrentState = IDLE;
                    return true;
                }

                case TRANSITION_DESCEND: {
                    if ("IS_PLAYER_SWITCH_IN_PROGRESS"_n())
                        break;

                    Rainbomizer::Logger::LogMessage (
                        "Player descent finished: %d",
                        *MissionRandomizer_GlobalsManager::PP_CURRENT_PED);

                    m_nCurrentState = IDLE;
                    return true;
                }
            }

        return false;
    }
};
