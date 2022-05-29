#pragma once

#include <algorithm>
#include <rage.hh>
#include <cstdint>
#include <cstddef>

enum class audSoundType : uint8_t
{
    audLoopingSound               = 1,
    audEnvelopeSound              = 2,
    audTwinLoopSound              = 3,
    audSpeechSound                = 4,
    audOnStopSound                = 5,
    audWrapperSound               = 6,
    audSequentialSound            = 7,
    audStreamingSound             = 8,
    audRetriggeredOverlappedSound = 9,
    audCrossfadeSound             = 10,
    audCollapsingStereoSound      = 11,
    audSimpleSound                = 12,
    audMultitrackSound            = 13,
    audRandomizedSound            = 14,
    audEnvironmentSound           = 15,
    audDynamicEntitySound         = 16,
    audSequentialOverlapSound     = 17,
    audModularSynthSound          = 18,
    audGranularSound              = 19,
    audDirectionalSound           = 20,
    audKineticSound               = 21,
    audSwitchSound                = 22,
    audVariableCurveSound         = 23,
    audVariablePrintValueSound    = 24,
    audVariableBlockSound         = 25,
    audIfSound                    = 26,
    audMathOperationSound         = 27,
    audParameterTransformSound    = 28,
    audFluctuatorSound            = 29,
    audAutomationSound            = 30,
    audExternalStreamSound        = 31,
    audSoundSet                   = 32,
};

namespace rage {
struct Sound
{
    audSoundType Type;
    uint32_t     DataFlags;
    uint32_t     Flags = 0xaaaaaaaa;
    uint16_t     field_0x9;
    int16_t      Volume;
    uint16_t     VolumeVariance;
    int16_t      Pitch;
    uint16_t     PitchVariance;
    int16_t      Pan;
    uint16_t     PanVariance;
    uint16_t     Predelay;
    uint16_t     PredelayVariance;
    int32_t      StartOffset;
    int32_t      StartOffsetVariance;
    uint16_t     AttackTime;
    int16_t      HoldTime;
    int16_t      ReleaseTime;
    uint32_t     Category;
    uint16_t     field_0x2d;
    uint16_t     field_0x2f;
    uint16_t     field_0x31;
    uint16_t     field_0x33;
    uint32_t     VolumeCurve;
    int16_t      VolumeCurveScale;
    uint8_t      field_0x3b;
    uint8_t      SpeakerMask;
    uint8_t      field_0x3d;
    int32_t      VolumeVariable;
    int32_t      PitchVariable;
    int16_t      field_0x46;
    int16_t      field_0x48;
    uint8_t      field_0x4a;
};
} // namespace rage

class audSound
{
public:
    static void *DecompressMetadata_Untyped (rage::Sound *inp,
                                             rage::Sound *out);
};

struct audMetadataObjectMapItem
{
    uint32_t m_nObjectOffset;
    uint32_t m_nObjectName;
};

struct audMetadataRef
{
    uint32_t Offset;
};
static_assert (sizeof (audMetadataRef) == 4, "audMetadataRef size incorrect");

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

    // These fields were added in v1.0.2189.0
    atArray<> m_RadioMusicFiles;
    uint32_t  field_0x30;

    uint32_t m_nNumStringsInStringTable;
    char *   m_pStringTableStart;
    char *   m_pStringTableEnd;
    uint64_t field_0x50;
    uint32_t field_0x58;
    int32_t  m_nStringTableSize;
    bool     m_bInitialised;

    // For iterating over the chunk
    template <typename T, typename F>
    void
    ForEach (F func)
    {
        for (uint32_t i = 0; i < m_nObjectMapSize; i++)
            func ((T *) ((char *) m_pObjectData
                         + m_pObjectMap[i].m_nObjectOffset),
                  m_pObjectMap[i].m_nObjectName, i);
    }

    uint32_t
    SearchForTypedObjectIndexFromHash (uint32_t name)
    {
        for (uint32_t i = 0; i < m_nObjectMapSize; i++)
            if (m_pObjectMap[i].m_nObjectName == name)
                return i;
        return -1;

#ifdef BINARY_SEARCH
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
#endif
    }

    template <typename T>
    T *
    FindObjectPtrFromHash (uint32_t name)
    {
        uint32_t index = SearchForTypedObjectIndexFromHash (name);
        if (index == -1u)
            return nullptr;
        return (T *) ((char *) m_pObjectData
                      + m_pObjectMap[index].m_nObjectOffset);
    }

    uint32_t
    FindObjectHashFromOffset (uint32_t offset)
    {
        for (uint32_t i = 0; i < m_nObjectMapSize; i++)
            if (m_pObjectMap[i].m_nObjectOffset == offset)
                return m_pObjectMap[i].m_nObjectName;
        return -1;
    }

    bool
    DoesObjectExist (uint32_t name)
    {
        return FindObjectPtrFromHash<void> (name) != nullptr;
    }

    static uint32_t GetSize ();
};

struct audMetadataMgr
{
    atArrayGetSizeWrapper<audMetadataChunk> Chunks;
    uint8_t                                 field_0xc[52];

    // For iterating over all the metadatas
    template <typename T, typename F>
    void
    ForEach (F func)
    {
        for (size_t i = 0; i < Chunks.Size; i++)
            Chunks[i].ForEach<T> (func);
    }

    bool
    DoesObjectExist (uint32_t name)
    {
        for (auto &i : Chunks)
            if (i.DoesObjectExist (name))
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
                ret = Chunks[i].FindObjectPtrFromHash<T> (name);
                if (ret)
                    break;
            }

        return ret;
    }

    void *FindObjectPtr (audMetadataRef ref);

    template <typename T>
    T *
    FindObjectPtr (audMetadataRef ref)
    {
        return (T *) FindObjectPtr (ref);
    }

    uint32_t
    FindObjectHashFromOffset (uint32_t offset)
    {
        if ((offset & 0xffffff) == 0xffffff)
            return -1;

        for (auto &i : Chunks)
            {
                uint32_t hash = i.FindObjectHashFromOffset (offset);
                if (hash != -1)
                    return hash;
            }
        return -1;
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
