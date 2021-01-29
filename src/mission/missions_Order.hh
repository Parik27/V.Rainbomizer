#pragma once

#include "mission/missions_Data.hh"
#include "mission/missions_Globals.hh"
#include "scrThread.hh"
#include "missions_Globals.hh"

#include <cstdint>
#include <map>
#include "Utils.hh"

struct MissionInfo
{
    uint32_t           nHash;
    uint32_t           nId;
    MissionDefinition *pDef;
    MissionDefinition  DefCopy;
    MissionData        Data;
};

class MissionRandomizer_OrderManager
{
    std::map<uint32_t, uint32_t> m_MissionsMap; // Original -> Randomized

    std::map<uint32_t, MissionInfo> m_MissionInfos;
    // To convert a gMissions index to original hash.

    void InitialiseMissionsMap (unsigned int seed);
    void Update_gMissions ();

    bool bInitialised = false;

public:
    void
    Reset ()
    {
        bInitialised = false;
        m_MissionInfos.clear ();
        m_MissionsMap.clear ();
    }

    bool
    IsInitialised ()
    {
        return bInitialised;
    }

    const MissionInfo *
    GetMissionInfo (uint32_t hash)
    {
        return LookupMap (m_MissionInfos, hash);
    }

    const MissionInfo *
    GetMissionInfoFromId (uint32_t id)
    {
        for (auto &[hash, info] : m_MissionInfos)
            {
                if (info.nId == id)
                    return &info;
            }

        return nullptr;
    }

    uint32_t
    GetRandomMissionHash (uint32_t nOriginalMission)
    {
        if (auto hash = LookupMap (m_MissionsMap, nOriginalMission))
            return *hash;

        return -1u;
    }

    void Process (scrThreadContext *ctx, scrProgram *program);
};
