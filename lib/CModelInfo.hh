#pragma once

#include <cstdint>
#include <cstddef>
#include <CMath.hh>

/* Note: offsets in this file are not to be used directly since they might not
 * be correct for all versions of the game. Using the functions makes it easier
 * to support future versions and also guarantees that it's going to work for at
 * least all the versions so far. Use offsets only for reference. The functions
 * marked as "confirmed" are checked for a wide range of versions and should
 *  work with any future updates

   (Applicable for CPedModelInfo, CVehicleModelInfo and CWeaponModelInfo)
*/

enum eModelInfoType : unsigned char
{
    MODEL_INFO_BASE    = 1,
    MODEL_INFO_MLO     = 2,
    MODEL_INFO_PED     = 6,
    MODEL_INFO_TIME    = 3,
    MODEL_INFO_VEHICLE = 5,
    MODEL_INFO_WEAPON  = 4
};

class CBaseModelInfo
{
public:
    void *        vft;
    uint8_t       __pad08[16];
    uint32_t      m_nHash;
    uint8_t       __pad1C[20];
    Vector4       m_vecMin;
    Vector4       m_vecMax;
    uint8_t       __padCC[77];
    unsigned char m_nType;
    uint8_t       __pad9E[18];

    eModelInfoType
    GetType ()
    {
        return (eModelInfoType) (m_nType & 31);
    };
};

class CVehicleModelInfo : public CBaseModelInfo
{
public:
    uint32_t GetVehicleType ();
    char* GetGameName ();
};

class CPedModelInfo : public CBaseModelInfo
{
public:
    uint8_t field_0xb0[54];
    uint32_t m_nMovementClipSet;
    uint8_t field_0xf4[4];
    uint64_t field_0xf8;
    uint32_t field_0x100;
    uint8_t field_0x104[4];
    uint32_t m_nStrafeClipSet;
    uint32_t m_nMovementToStrafeClipSet;
    uint32_t m_nInjuredStrafeClipSet;
    uint32_t m_nFullBodyDamageClipSet;
    uint32_t m_nAdditiveDamageClipSet;
    uint32_t m_nDefaultGestureClipSet;
    uint32_t m_nDefaultVisemeClipSet;
    uint32_t m_nSidestepClipSet;
    uint32_t m_nExperssionSetName;
    uint32_t m_nFacialClipsetGroupName;
    uint32_t GetupSetHash;
    bool m_bIsPlayerType; // confirmed
    uint8_t field_0x135[3];
    uint32_t m_nPedType; // confirmed
    float m_nMinActivationImpulse;
    float m_fStubble;
    uint8_t field_0x144[6];
    short field_0x14a;
    uint8_t field_0x14c[16];
    uint8_t Radio1;
    uint8_t Radio2;
    uint8_t field_0x15e[2];
    float m_fFUpOffset;
    float m_fFFrontOffset;
    float m_fRUpOffset;
    float m_fRFrontOffset;
    uint8_t field_0x170[2];
    short field_0x172;
    short field_0x174;
    short field_0x176;
    uint8_t field_0x178[4];
    uint32_t m_nPedVoiceGroup;
    uint32_t m_nAnimalAudioObject;
    uint8_t field_0x184[4];
    uint32_t m_nSexiness;
    uint8_t field_0x18c[4];
    uint8_t m_bAge;
    uint8_t m_nMaxPassengersInCar;
    uint8_t m_bExternallyDrivenDOFs;
    uint8_t field_0x193[57];
    uint32_t m_nScenarioPopStreamingSlot;
    uint32_t m_nDefaultSpawningPreference;
    float m_fDefaultRemoveRangeMultiplier;
    bool m_bAllowCloseSpawning;
    uint8_t field_0x1ed[11];
    char * m_pVfxInfoName;
    uint8_t field_0x200[8];
    float m_fHDDist;
    uint8_t field_0x20c[5];
    float m_fTargetingThreadModifier;
    float m_fKilledPerceptionRangeModifier;
    uint64_t field_0x220[6];
    uint32_t m_nAmbientClipsForFlee;
    uint8_t field_0x234[12];
    uint32_t m_nMotionTaskDataSetName;
    uint32_t m_nDefaultTaskDataSetName;
    uint32_t m_nPedCapsuleName;
    uint32_t m_nPedLayoutName;
    uint32_t m_nPedComponentSetName;
    uint32_t m_nPedComponentClothName;
    uint32_t m_nPedIKSettingsName;
    uint32_t m_nTaskDataName;
    uint32_t m_nDecisionMakerName;
    uint32_t m_nRelationshipGroup;
    uint32_t m_nNewCapabilitiesName;
    uint32_t m_nPerceptionInfo;
    uint32_t m_nDefaultBrawlingStyle;
    uint32_t m_nDefaultUnarmedWeapon;
    uint8_t field_0x278[6];
    uint8_t m_nFlags;
    uint8_t field_0x27f;
    uint8_t m_nFlags2;
    uint8_t field_0x281[15];
};

static_assert (sizeof (CBaseModelInfo) == 176, "Incorect BaseModelInfo size");
