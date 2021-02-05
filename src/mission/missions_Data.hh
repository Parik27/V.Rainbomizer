#pragma once

#include "Utils.hh"
#include "common/logger.hh"
#include "rage.hh"
#include <cstdint>
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include <map>

#include "missions_Globals.hh"

struct MissionData
{

    struct Coords : rage::Vec3V
    {
        std::vector<ePlayerIndex> ValidPlayers;
        ePlayerIndex
        GetRandomPlayer () const
        {
            return GetRandomElement (ValidPlayers);
        }
    };

    char sName[32] = {0};
    char sGXT[16] = {0};
    char sCutscene[64] = {0};
    
    Coords vecStartCoords;
    Coords vecEndCoords;
    
    bool
    Read (const char *line)
    {
        char startPlayer[4] = {0};
        char endPlayer[4]   = {0};

        if (sscanf (line, " %32s %16s %64s %f %f %f %3s %f %f %f %3s ", sName,
                    sGXT, sCutscene, &vecStartCoords.x, &vecStartCoords.y,
                    &vecStartCoords.z, startPlayer, &vecEndCoords.x,
                    &vecEndCoords.y, &vecEndCoords.z, endPlayer)
            < 10)
            return false;

        auto ReadPlayerBitset = [] (decltype(startPlayer) str) {
            std::vector<ePlayerIndex> b;

            if(str[0] == 'M' || str[1] == 'M' || str[2] == 'M')
                b.push_back(ePlayerIndex::PLAYER_MICHAEL);
            if(str[0] == 'T' || str[1] == 'T' || str[2] == 'T')
                b.push_back(ePlayerIndex::PLAYER_TREVOR);
            if (str[0] == 'F' || str[1] == 'F' || str[2] == 'F')
                b.push_back(ePlayerIndex::PLAYER_FRANKLIN);

            return b;
        };

        vecStartCoords.ValidPlayers = ReadPlayerBitset (startPlayer);
        vecEndCoords.ValidPlayers   = ReadPlayerBitset (endPlayer);
        return true;
    }
};

class MissionRandomizer_Data
{
    std::map<uint32_t, MissionData> m_MissionData;

public:

    bool
    IsValidMission (uint32_t missionName)
    {
        return m_MissionData.count (missionName);
    }

    const auto&
    GetMissionDataMap ()
    {
        return m_MissionData;
    }

    const auto &
    GetMissionData (uint32_t hash)
    {
        return m_MissionData.at (hash);
    }

    MissionRandomizer_Data (FILE *file)
    {
        char line[1024] = {0};
        while (fgets (line, 1024, file))
            {
                if (strlen (line) < 10 || line[0] == '#')
                    continue;

                // Read Mission Data
                MissionData m;
                if (m.Read (line))
                    m_MissionData[rage::atStringHash (m.sName)] = m;
                else
                    Rainbomizer::Logger::LogMessage (
                        "Error reading Missions.txt line: %s", line);
            }
    }
};
