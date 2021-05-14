#include "missions_Order.hh"
#include "common/logger.hh"
#include "mission/missions_Globals.hh"
#include "missions.hh"

#include "rage.hh"

#include <cstdint>
#include <map>
#include <string.h>
#include <vector>
#include <random>

#ifdef ENABLE_DEBUG_SERVER
#include "debug/actions.hh"
#endif

using MR = MissionRandomizer_Components;

MissionRandomizer_OrderManager::SaveStructure *
MissionRandomizer_OrderManager::GetSaveStructure ()
{
    return static_cast<SaveStructure *> (MR::sm_Globals.MF_MISSION_STRUCT_99);
}

bool
MissionRandomizer_OrderManager::Process (scrProgram *      program,
                                         scrThreadContext *ctx)
{
    if (program->m_nScriptHash == "initial"_joaat
        && ctx->m_nState == eScriptState::KILLED)
        {
            InitialiseMissionsMap (GetSeed ());
            Update_gMissions ();

            bInitialised = true;
        }

    if (program->m_nScriptHash == "flow_controller"_joaat && ctx->m_nIp == 0)
        MR::sm_Cmds.AdjustMissionFlowCommands ();

#ifdef ENABLE_DEBUG_SERVER
    if (ActionsDebugInterface::sm_ReloadMissionsRequested)
        {
            uint32_t seed = static_cast<uint32_t> (
                std::hash<std::string>{}(MR::Config ().Seed));

            InitialiseMissionsMap (seed);
            Update_gMissions ();
            ActionsDebugInterface::sm_ReloadMissionsRequested = false;
        }
#endif

    return true;
}

uint32_t
MissionRandomizer_OrderManager::GetSeed ()
{
    auto *save = GetSaveStructure ();

    uint32_t randomSeed = RandomSize (UINT_MAX);
    uint32_t configSeed = rage::atLiteralStringHash (MR::Config ().Seed);

    if (!save->ValidateSaveStructure ())
        *save = SaveStructure (configSeed == 0 ? randomSeed : configSeed);

    return MR::Config ().ForceSeedOnSaves ? configSeed : save->Seed;
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

    m_Choices.AgencyFiretruck = engine () % 2;
    m_Choices.DocksBlowUpBoat = engine () % 2;
    m_Choices.FinaleHeli      = engine () % 2;
    m_Choices.JewelStealth    = engine () % 2;

    // Print information about the seed.
#define PRINT_CHOICE(choice, yes, no)                                          \
    Rainbomizer::Logger::LogMessage (#choice ": %s",                           \
                                     m_Choices.choice ? yes : no)

    Rainbomizer::Logger::LogMessage ("Mission Randomizer Initialised.");
    Rainbomizer::Logger::LogMessage ("Seed: %u (%x)", seed, seed);
    PRINT_CHOICE (AgencyFiretruck, "AGENCY_FIRETRUCK", "AGENCY_HELICOPTER");
    PRINT_CHOICE (DocksBlowUpBoat, "DOCKS_BLOW_UP_BOAT", "DOCKS_DEEP_SEA");
    PRINT_CHOICE (FinaleHeli, "FINALE_HELI", "FINALE_TRAFFCONT");
    PRINT_CHOICE (JewelStealth, "JEWEL_STEALTH", "JEWEL_HIGH_IMPACT");

#undef PRINT_CHOICE
}

void
MissionRandomizer_OrderManager::Update_gMissions ()
{
    auto g_Missions = MR::sm_Globals.g_Missions.Get ();
    if (m_MissionInfos.empty ())
        {
            for (unsigned int i = 0; i < g_Missions->Size; i++)
                {
                    auto &data = g_Missions->Data[i];
                    if (!MR::sm_Data.IsValidMission (data.nThreadHash))
                        continue;

                    m_MissionInfos[data.nThreadHash]
                        = {data.nThreadHash, i, &data, data,
                           MR::sm_Data.GetMissionData (data.nThreadHash)};
                }
        }

    for (unsigned int i = 0; i < g_Missions->Size; i++)
        {
            auto &origMission = g_Missions->Data[i];

            if (!m_MissionsMap.count (origMission.nThreadHash))
                continue;

            uint32_t newHash    = m_MissionsMap[origMission.nThreadHash];
            auto &   newMission = GetMissionInfo (newHash)->DefCopy;

            strncpy (origMission.sMissionThread, newMission.sMissionThread, 24);
            origMission.nThreadHash = newMission.nThreadHash;

            origMission.BITS_MissionFlags.NO_STAT_WATCHER
                = newMission.BITS_MissionFlags.NO_STAT_WATCHER;
            origMission.BITS_MissionFlags.DISABLE_MISSION_SKIPS
                = newMission.BITS_MissionFlags.DISABLE_MISSION_SKIPS;
        }
}

void
MissionRandomizer_OrderManager::RestoreOriginalMissionInfo (uint32_t missionId)
{
    auto info = GetMissionInfoFromId (missionId);
    if (!info || !info->pDef)
        return;

    m_StoredRandomInfo                         = *info->pDef;
    MR::sm_Globals.g_Missions->Data[missionId] = info->DefCopy;
}

void
MissionRandomizer_OrderManager::ReapplyRandomMissionInfo (uint32_t missionId)
{
    MR::sm_Globals.g_Missions->Data[missionId] = m_StoredRandomInfo;
}
