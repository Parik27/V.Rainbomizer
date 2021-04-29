#include "peds_Compatibility.hh"

#include "CModelInfo.hh"
#include "Utils.hh"
#include <CPed.hh>
#include <CStreaming.hh>

#include <common/logger.hh>

/*******************************************************/
CPedModelInfo *
PedRandomizerCompatibility::GetOriginalModel (CPed *ped)
{
    auto *model = ped->m_pModelInfo;
    if (!model)
        return static_cast<CPedModelInfo *> (model);

    if (auto *data = LookupMap (sm_PedModelFixupMap, ped))
        {
            auto *fromModel = CStreaming::GetModelByIndex (data->first);
            auto *toModel   = CStreaming::GetModelByIndex (data->second);

            if (toModel->m_nHash == model->m_nHash)
                model = fromModel;
        }

    return static_cast<CPedModelInfo *> (model);
}

/*******************************************************/
void
PedRandomizerCompatibility::AddRandomizedPed (CPed *ped, uint32_t from,
                                              uint32_t to)
{
    sm_PedModelFixupMap[ped] = std::make_pair (from, to);

    auto *model
        = static_cast<CPedModelInfo *> (CStreaming::GetModelByIndex (from));

    if (IsPlayerModel (model->m_nHash))
        {
            sm_ModelMap[model->m_nHash]
                = CStreaming::GetModelByIndex (to)->m_nHash;

            Rainbomizer::Logger::LogMessage (
                "Randomized Player Model: %x -> %x", model->m_nHash,
                sm_ModelMap[model->m_nHash]);
        }
}

bool
PedRandomizerCompatibility::IsPlayerModel (uint32_t model)
{
    switch (model)
        {
        case "player_zero"_joaat:
        case "player_one"_joaat:
        case "player_two"_joaat: return true;

        default: return false;
        }
}
