#pragma once

#include "mission/missions_Globals.hh"
#include "scrThread.hh"
#include "missions_Globals.hh"

#include <cstdint>
#include <map>
#include "Utils.hh"

class MissionRandomizer_OrderManager
{
    std::map<uint32_t, uint32_t>
        m_MissionsMap; // Original -> Randomized

    std::map<uint32_t, uint32_t> m_gMissionsTranslation;
    // To convert a gMissions index to original hash.

    void InitialiseMissionsMap (unsigned int seed);
    void Update_gMissions ();

    bool bInitialised = false;

public:
    void
    Reset ()
    {
        bInitialised = false;
        m_gMissionsTranslation.clear ();
        m_MissionsMap.clear ();
    }

    bool
    IsInitialised ()
    {
        return bInitialised;
    }

    uint32_t
    GetOriginalMissionHash (uint32_t gMissionsIndex)
    {
        if (auto hash = LookupMap (m_gMissionsTranslation, gMissionsIndex))
            return *hash;

        return -1u;
    }

    uint32_t
    GetRandomMissionHash (uint32_t nOriginalMission)
    {
        if (auto hash = LookupMap (m_MissionsMap, nOriginalMission))
            return *hash;

        return -1u;
    }

    MissionDefinition *GetDefinitionByHash (uint32_t hash);
    
    void Process (scrThreadContext *ctx, scrProgram *program);
};
