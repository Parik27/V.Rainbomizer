#include "missions_Order.hh"
#include "mission/missions_Globals.hh"
#include "missions.hh"

#include "rage.hh"

#include <cstdint>
#include <map>
#include <string.h>
#include <vector>
#include <random>

using MR = MissionRandomizer_Components;

void
MissionRandomizer_OrderManager::Process (scrThreadContext *ctx,
                                         scrProgram *      program)
{
    if (program->m_nScriptHash == "standard_global_init"_joaat
        && ctx->m_nState == eScriptState::KILLED)
        {
            uint32_t seed = static_cast<uint32_t> (
                std::hash<std::string>{}(MR::Config ().Seed));

            InitialiseMissionsMap (seed);
            Update_gMissions ();

            bInitialised = true;
        }
}

void
MissionRandomizer_OrderManager::InitialiseMissionsMap (unsigned int seed)
{
    std::vector<uint32_t> missions;
    std::vector<uint32_t> order;
    std::mt19937          engine{seed};

    for (const auto &s : MR::sm_Data.GetMissionDataMap ())
        missions.push_back (s.first);

    order = missions;
    std::shuffle (order.begin (), order.end (), engine);

    uint32_t forcedHash = -1u;
    if (MR::Config ().ForcedMission.size ())
        forcedHash = rage::atStringHash (MR::Config ().ForcedMission);

    for (unsigned int i = 0; i < missions.size (); i++)
        {
            if (forcedHash == -1u)
                m_MissionsMap[missions[i]] = order[i];
            else
                m_MissionsMap[missions[i]] = forcedHash;
        }
}

MissionDefinition *
MissionRandomizer_OrderManager::GetDefinitionByHash (uint32_t hash)
{
    for (unsigned int i = 0; i < MR::sm_Globals.g_Missions.nSize; i++)
        {
            auto &mission = MR::sm_Globals.g_Missions.Data[i];
            if (mission.nThreadHash == hash)
                return &mission;
        }

    return nullptr;
}

void
MissionRandomizer_OrderManager::Update_gMissions ()
{
    for (unsigned int i = 0; i < MR::sm_Globals.g_Missions.nSize; i++)
        {
            auto &origMission = MR::sm_Globals.g_Missions.Data[i];

            if (!m_MissionsMap.count (origMission.nThreadHash))
                continue;

            auto *newMission = GetDefinitionByHash (
                m_MissionsMap[origMission.nThreadHash]);

            if (!newMission)
                continue;

            m_gMissionsTranslation[i] = origMission.nThreadHash;

            strncpy (origMission.sMissionThread, newMission->sMissionThread,
                     24);
            origMission.nThreadHash = newMission->nThreadHash;
        }
}
