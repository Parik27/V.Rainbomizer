#pragma once

#include <CModelInfo.hh>

struct modelInfoArray
{
    CBaseModelInfo** m_pData;
    uint64_t m_nAllocated;
};

/* Struct representing loaded? models in CStreaming::sm_Instance (not actual
 * name) */
struct CStreaming_LoadedModel
{
    uint32_t mModelIndex;
    int32_t  field_0x4;
    int32_t  field_0x8;
    int32_t  field_0xc;
    int32_t  field_0x10;
    uint8_t  field_0x14;
    bool     m_bIsPedModel;
    uint8_t  field_0x16;
    uint8_t field_0x17;
};

class CModelList
{
public:
    uint32_t m_aMembers[256];
    
    template <class Functor>
    void
    for_each (Functor f)
    {
        const uint32_t TOTAL_MODEL_POINTERS = 0xF230; // From game exe
        for (auto i : m_aMembers)
            {
                if (i == 65535) break;
                if (i >= TOTAL_MODEL_POINTERS) break;

                f (i);
            }
    }
};

// + CStreaming_Population
class CStreaming
{
public:
    CStreaming_LoadedModel mLoadedModels[480];
    CModelList             mAppropriateCarsSet;
    CModelList             mInappropriateCarsSet;
    CModelList             mSpecialVehiclesSet;
    CModelList             mBoatsSet;
    CModelList             mMultiplayerAppropriateSet;
    CModelList             mVehiclesSet6;
    bool                   m_bSpawnBoats;
    uint8_t                field_0x4501;
    uint8_t                field_0x4502;
    uint8_t                field_0x4503;
    CModelList             mAppropriatePedsSet;
    CModelList             mInAppropriatePedsSet;
    CModelList             mCopsSet;
    CModelList             mMultiplayerPedsSet;
    CModelList             mPedsSet5;
    int                    field_0x5904;
    int                    field_0x5908;

    static void InitialisePatterns ();

    static bool HasModelLoaded (uint32_t modelId);
    static void RequestModel (uint32_t modelId, uint32_t flags);
    static void LoadAllObjects (bool priorityOnly);
    
    static CBaseModelInfo* GetModelAndIndexByHash (uint32_t hash, uint32_t& outIndex);

    static inline uint32_t GetModelIndex (uint32_t hash)
    {
        uint32_t index = -1;
        GetModelAndIndexByHash(hash, index);

        return index;
    }

    template <typename T>
    static inline T *
    GetModelAndIndexByHash (uint32_t hash, uint32_t &outIndex)
    {
        return static_cast<T *> (GetModelAndIndexByHash (hash, outIndex));
    }

    template <typename T>
    static inline T *
    GetModelByHash (uint32_t hash)
    {
        uint32_t index = 0;
        return GetModelAndIndexByHash<T> (hash, index);
    }
    
    template <typename T>
    static inline T*
    GetModelByIndex (uint32_t index)
    {
        return static_cast<T *>(ms_aModelPointers->m_pData[index]);
    }
    
    static modelInfoArray* ms_aModelPointers;
    static CStreaming* sm_Instance;
};
