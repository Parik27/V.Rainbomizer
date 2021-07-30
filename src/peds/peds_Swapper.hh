#pragma once

#include "CModelInfo.hh"
#include "Utils.hh"
#include <CStreaming.hh>
#include <cstdint>
#include <map>
#include <rage.hh>
#include <string_view>

class PedRandomizer_ModelSwapper
{
    uint32_t m_IdA;
    uint32_t m_IdB;

    struct ClipsetInfo
    {
        uint32_t MovementClipset;
        uint32_t MotionTaskDataSetName;
    };
    inline static std::map<uint32_t, ClipsetInfo> sm_ClipsetInfos;

    /*******************************************************/
    static void
    SwapModels (uint32_t a, uint32_t b)
    {
        auto modelA = CStreaming::GetModelByIndex<CPedModelInfo> (a);
        auto modelB = CStreaming::GetModelByIndex<CPedModelInfo> (b);

#define SWAP_FIELD(field) std::swap (modelA->field, modelB->field);

        SWAP_FIELD (m_nCombatInfo);
        SWAP_FIELD (m_nPedType);
        SWAP_FIELD (m_nPersonality);

        SWAP_FIELD (GetInitInfo ().m_nTaskDataName);
        SWAP_FIELD (GetInitInfo ().m_nRelationshipGroup);
        SWAP_FIELD (GetInitInfo ().m_nDecisionMakerName);
        SWAP_FIELD (GetInitInfo ().m_nNavCapabilitiesName);

#undef SWAP_FIELD
    }

    /*******************************************************/
    static ClipsetInfo *
    GetAndStoreClipsetInfoForModel (CPedModelInfo *model)
    {
        if (!model)
            return nullptr;

        auto [iterator, added] = sm_ClipsetInfos.emplace (
            model->m_nHash,
            ClipsetInfo{model->m_nMovementClipSet,
                        model->GetInitInfo ().m_nMotionTaskDataSetName});

        return &iterator->second;
    }

    /*******************************************************/
    static ClipsetInfo *
    GetAndStoreClipsetInfoForModel (uint32_t hash)
    {
        return GetAndStoreClipsetInfoForModel (
            CStreaming::GetModelByHash<CPedModelInfo> (hash));
    }

public:
    /*******************************************************/
    PedRandomizer_ModelSwapper (uint32_t idA, uint32_t idB)
        : m_IdA (idA), m_IdB (idB)
    {
        SwapModels (idA, idB);
    }

    /*******************************************************/
    static void
    SetClipset (uint32_t clipset, uint32_t modelId)
    {
        auto *model = CStreaming::GetModelByIndex<CPedModelInfo> (modelId);

        GetAndStoreClipsetInfoForModel (model);
        ClipsetInfo *newClipset = GetAndStoreClipsetInfoForModel (clipset);

        if (!model || !newClipset)
            return;

        model->m_nMovementClipSet = newClipset->MovementClipset;
        model->GetInitInfo ().m_nMotionTaskDataSetName
            = newClipset->MotionTaskDataSetName;
    }

    /*******************************************************/
    void
    OverrideClipset (std::string_view newClipset) const
    {
        if (newClipset.empty ())
            return;

        uint32_t clipsetHash = rage::atStringHash (newClipset);
        SetClipset (clipsetHash, m_IdA);
        SetClipset (clipsetHash, m_IdB);
    }

    ~PedRandomizer_ModelSwapper () { SwapModels (m_IdA, m_IdB); }
};
