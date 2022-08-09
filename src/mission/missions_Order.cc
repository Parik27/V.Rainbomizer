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

#ifdef ENABLE_DEBUG_MENU
#include "debug/base.hh"
#endif

using MR = MissionRandomizer_Components;

MissionRandomizer_OrderManager::SaveStructure *
MissionRandomizer_OrderManager::GetSaveStructure ()
{
    return static_cast<SaveStructure *> (MR::sm_Globals.MF_MISSION_STRUCT_99);
}

bool
MissionRandomizer_OrderManager::Process (scrProgram       *program,
                                         scrThreadContext *ctx)
{
    auto Initialise = [this] (uint32_t Seed) {
        if (!MR::Config ().ForcedOrder.size ()
            || !InitialiseMissionsMap (MR::Config ().ForcedOrder))
            InitialiseMissionsMap (Seed);

        Update_gMissions ();
        bInitialised = true;
    };

    if (program->m_nScriptHash == "initial"_joaat
        && ctx->m_nState == eScriptState::KILLED)
        {
            Initialise (GetSeed ());
        }

    if (program->m_nScriptHash == "flow_controller"_joaat && ctx->m_nIp == 0)
        MR::sm_Cmds.AdjustMissionFlowCommands ();

#ifdef ENABLE_DEBUG_MENU
    if (DebugInterfaceManager::GetAction ("Reload Missions"))
        {
            uint32_t seed = static_cast<uint32_t> (
                std::hash<std::string>{}(MR::Config ().Seed));

            Initialise (seed);
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

bool
MissionRandomizer_OrderManager::ValidateOrderString (std::string_view order)
{
    const int NUM_CHOICES = 4;

    if (order.size () != MR::sm_Data.GetMissionDataMap ().size () + NUM_CHOICES)
        return false;

    for (int i = 0; i < order.size () - 4; i++)
        {
            if (order[i] < ORD_STRING_START || order[i] > ORD_STRING_END)
                return false;
        }

    return true;
}

bool
MissionRandomizer_OrderManager::InitialiseMissionsMap (std::string_view order)
{
    if (!ValidateOrderString (order))
        {
            Rainbomizer::Logger::LogMessage (
                "Failed to validate order string: %s", order.data ());
            return false;
        }

    auto &orderMap = MR::sm_Data.GetMissionOrderMap ();

    for (unsigned int i = 0; i < orderMap.size (); i++)
        {
            m_MissionsMap[orderMap[i]]
                = MR::sm_Data.GetMissionHashFromOrderChar (order[i]
                                                           - ORD_STRING_START);
        }

    m_Choices.AgencyFiretruck = order[order.size () - 1] == '1';
    m_Choices.DocksBlowUpBoat = order[order.size () - 2] == '1';
    m_Choices.FinaleHeli      = order[order.size () - 3] == '1';
    m_Choices.JewelStealth    = order[order.size () - 4] == '1';

    Rainbomizer::Logger::LogMessage ("Mission Randomizer Initialised.");
    Rainbomizer::Logger::LogMessage ("Applied Forced Order: %s",
                                     GenerateOrderString ().c_str ());

    return true;
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
    
    Rainbomizer::Logger::LogMessage ("Order String: %s",
                                     GenerateOrderString ().c_str ());
}

std::string
MissionRandomizer_OrderManager::GenerateOrderString ()
{
    std::string order;
    auto       &orderMap = MR::sm_Data.GetMissionOrderMap ();

    for (unsigned int i = 0; i < orderMap.size (); i++)
        order += MR::sm_Data.GetOrderCharFromMissionHash (
                     m_MissionsMap[orderMap[i]])
                 + ORD_STRING_START;

    order += m_Choices.JewelStealth ? '1' : '0';
    order += m_Choices.FinaleHeli ? '1' : '0';
    order += m_Choices.DocksBlowUpBoat ? '1' : '0';
    order += m_Choices.AgencyFiretruck ? '1' : '0';

    return order;
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
