#pragma once

#include "common/logger.hh"
#include "memory/GameAddress.hh"
#include <CModelInfo.hh>

struct modelInfoArray
{
    CBaseModelInfo **m_pData;
    uint64_t         m_nAllocated;
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
    uint8_t  field_0x17;
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
                if (i == 65535)
                    break;
                if (i >= TOTAL_MODEL_POINTERS)
                    break;

                f (i);
            }
    }
};

// + CStreaming_Population

class CStreamingModelSets
{
public:
    CModelList mAppropriateCarsSet;
    CModelList mInappropriateCarsSet;
    CModelList mSpecialVehiclesSet;
    CModelList mBoatsSet;
    CModelList mMultiplayerAppropriateSet;
    CModelList mVehiclesSet6;
    bool       m_bSpawnBoats;
    uint8_t    field_0x4501;
    uint8_t    field_0x4502;
    uint8_t    field_0x4503;
    CModelList mAppropriatePedsSet;
    CModelList mInAppropriatePedsSet;
    CModelList mCopsSet;
    CModelList mMultiplayerPedsSet;
    CModelList mPedsSet5;
};

class CStreaming
{
public:
    union
    {

        struct // and above
        {
            CStreaming_LoadedModel mLoadedModels[576];
            CStreamingModelSets    mModelSets;
            int                    field_0x5d04;
            int                    field_0x5d08;
        } b2944;

        struct {
            CStreaming_LoadedModel mLoadedModels[480];
            CStreamingModelSets    mModelSets;
            int                    field_0x5904;
            int                    field_0x5908;
        };
    };

    inline static GameFunction<100103, bool(const uint32_t&)> HasModelLoaded{};
    inline static GameFunction<100104, void(const uint32_t&, uint32_t)> RequestModel{};
    inline static GameFunction<100105, void(bool)> LoadAllObjects{};
    inline static GameFunction<100106, void(const uint32_t&)> DeleteModel{};

    static CStreamingModelSets* GetModelSets()
    {
        if (Rainbomizer::Logger::GetGameBuild() >= 2944)
            return &sm_Instance->b2944.mModelSets;
        return &sm_Instance->mModelSets;
    }

    static inline uint32_t
    GetModelIndex (uint32_t hash)
    {
        uint32_t index = -1;
        if (!GetModelAndIndexByHash (hash, index))
            return -1;

        return index & 0xFFFF;
    }

    static inline uint32_t
    GetModelHash (uint32_t index)
    {
        return GetModelByIndex (index)->m_nHash;
    }

    template <typename T = CBaseModelInfo>
    static inline T *
    GetModelAndIndexByHash (uint32_t hash, uint32_t &outIndex)
    {
        return static_cast<T *> (
            GameFunction<100107, CBaseModelInfo *(uint32_t, uint32_t &)>::Call (
                hash, outIndex));
    }

    template <typename T = CBaseModelInfo>
    static inline T *
    GetModelByHash (uint32_t hash)
    {
        uint32_t index = 0;
        return GetModelAndIndexByHash<T> (hash, index);
    }

    template <typename T = CBaseModelInfo>
    static inline T *
    GetModelByIndex (uint32_t index)
    {
        return static_cast<T *> (ms_aModelPointers->m_pData[index]);
    }

    inline static GameVariable<modelInfoArray, 100101> ms_aModelPointers;
    inline static GameVariable<CStreaming, 100102> sm_Instance;
};
