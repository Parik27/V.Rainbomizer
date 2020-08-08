#pragma once

#include <rage.hh>
#include <cstdint>
#include <cstddef>

struct audMetadataObjectMapItem
{
    uint32_t m_nObjectOffset;
    uint32_t m_nObjectName;
};

struct audMetadataChunk
{
    uint32_t                  m_nHash;
    char *                    m_sFileName;
    void *                    m_pObjectData;
    int32_t                   m_nObjectDataSize;
    uint32_t                  m_nObjectMapSize;
    audMetadataObjectMapItem *m_pObjectMap;
    void *                    field_0x28;
    char *                    m_apStringTable;
    uint32_t                  m_nNumStringsInStringTable;
    char *                    m_pStringTableStart;
    char *                    m_pStringTableEnd;
    uint64_t                  field_0x50;
    uint32_t                  field_0x58;
    int32_t                   m_nStringTableSize;
    bool                      m_bInitialised;

    // For iterating over the chunk
    template <typename T, typename F>
    void
    for_each (F func)
    {
        for (size_t i = 0; i < m_nObjectMapSize; i++)
            func ((T *) ((char *) m_pObjectData
                         + m_pObjectMap[i].m_nObjectOffset),
                  m_pObjectMap[i].m_nObjectName, i);
    }

    uint32_t
    SearchForTypedObjectIndexFromHash (uint32_t name)
    {
        for (size_t i = 0; i < m_nObjectMapSize; i++)
            if (m_pObjectMap[i].m_nObjectName == name)
                return i;
        return -1;
        
        if (m_nObjectMapSize == 0 || !m_pObjectMap || !m_pObjectData)
            return -1;
        
        size_t L = 0;
        size_t R = m_nObjectMapSize - 1;

        while (L <= R)
            {
                size_t m = (L + R) / 2;
                if (m_pObjectMap[m].m_nObjectName < name)
                    L = m + 1;
                else if (m_pObjectMap[m].m_nObjectName > name)
                    R = m - 1;
                else
                    return m;
            }
        return -1;
    }

    template <typename T>
    T *
    FindObjectPtrFromHash (uint32_t name)
    {
        uint32_t index = SearchForTypedObjectIndexFromHash (name);
        if (index == -1)
            return nullptr;
        return (T *) ((char *) m_pObjectData
                      + m_pObjectMap[index].m_nObjectOffset);
    }

    bool
    DoesObjectExist (uint32_t name)
    {
        return FindObjectPtrFromHash<void> (name) != nullptr;
    }
};

static_assert (sizeof (audMetadataChunk) == 104,
               "audMetadataChunk is wrong size check align.");
static_assert (offsetof (audMetadataChunk, m_bInitialised) == 96,
               "m_bInitialised at wrong offset, check align");

struct audMetadataMgr
{
    atArray<audMetadataChunk> Chunks;
    uint8_t                   field_0xc[52];

    // For iterating over all the metadatas
    template <typename T, typename F>
    void
    for_each (F func)
    {
        for (size_t i = 0; i < Chunks.Size; i++)
            Chunks.Data[i].for_each<T> (func);
    }

    bool
    DoesObjectExist (uint32_t name)
    {
        for (size_t i = 0; i < Chunks.Size; i++)
            if (Chunks.Data[i].DoesObjectExist(name))
                return true;
        
        return false;
    }

    template <typename T>
    T *
    FindObjectPtr (uint32_t name)
    {
        T *ret = nullptr;
        for (size_t i = 0; i < Chunks.Size; i++)
            {
                ret = Chunks.Data[i].FindObjectPtrFromHash<T> (name);
                if (ret)
                    break;
            }

        return ret;
    }

    // CRITICAL_SECTION m_Mutex;
    uint8_t m_Mutex[40];

    uint32_t m_nMetadataMgrName;
    int32_t  m_Suffix;
    int32_t  m_nType;
    bool     field_0x74;
};

class audSpeechManager
{
public:
    static audMetadataMgr *sm_MetadataMgr;
};

class audEngine
{
public:
    static void InitialisePatterns ();
};
