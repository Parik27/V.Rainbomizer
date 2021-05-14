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

    struct Choices
    {
        bool JewelStealth    = false;
        bool DocksBlowUpBoat = false;
        bool AgencyFiretruck = false;
        bool FinaleHeli      = false;
    } m_Choices;

    struct SaveStructure
    {
        alignas (8) char Signature[4] = {'R', 'B', 'M', 'Z'};
        alignas (8) unsigned int Seed;

        SaveStructure (unsigned int seed) : Seed (seed) {}

        bool
        ValidateSaveStructure () const
        {
            return Signature[0] == 'R' && Signature[1] == 'B'
                   && Signature[2] == 'M' && Signature[3] == 'Z';
        }
    };

    SaveStructure *GetSaveStructure ();
    uint32_t       GetSeed ();

    MissionDefinition m_StoredRandomInfo;

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

    auto
    GetChoices ()
    {
        return m_Choices;
    }

    void RestoreOriginalMissionInfo (uint32_t missionId);
    void ReapplyRandomMissionInfo (uint32_t missionId);

    bool Process (scrProgram *program, scrThreadContext *ctx);
};
