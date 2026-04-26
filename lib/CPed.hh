#pragma once

#include <cstdint>
#include <rage.hh>
#include "CEntity.hh"
#include "ParserUtils.hh"
#include "memory/GameAddress.hh"

class CInventoryItem;
class CPed;
class CVehicle;
struct aiTask;

class CPedWeaponManager
{
public:
    void *vft;
    void *param_0x8;
    CPed *m_pPed;
};

struct CInventoryItemRepository
{
    CInventoryItem **  m_pInventoryItems;
    short              m_nNumInventoryItems;
    uint8_t            field_0xa[6];
    CPedWeaponManager *m_pManagers[2];
    int                m_nNumManagers;

    CInventoryItem *FindWeapon (uint32_t hash)
    {
        return GameFunction<100097, CInventoryItem*(CInventoryItemRepository*, uint32_t)>::Call (this, hash);
    }
};

typedef enum eCombatBehaviourFlag
{
    BF_CanUseCover,
    BF_CanUseVehicles,
    BF_CanDoDrivebys,
    BF_CanLeaveVehicle,
    BF_CanUseDynamicStrafeDecisions,
    BF_AlwaysFight,
    BF_66bb9fcc,
    BF_6837da41,
    BF_b4a13a5a,
    BF_ee326aad,
    BF_7df2ccfa,
    BF_36d422,
    BF_BlindFireWhenInCover,
    BF_Aggressive,
    BF_CanInvestigate,
    BF_HasRadio,
    BF_6bde28d1,
    BF_AlwaysFlee,
    BF_7852797d,
    BF_33497b95,
    BF_CanTauntInVehicle,
    BF_4a0783d6,
    BF_4f8caec8,
    BF_cd7168b8,
    BF_UseProximityFiringRate,
    BF_DisableSecondaryTarget,
    BF_DisableEntryReactions,
    BF_PerfectAccuracy,
    BF_CanUseFrustratedAdvance,
    BF_3d131ac1,
    BF_3ad95f27,
    BF_MaintainMinDistanceToTarget,
    BF_ead68ad2,
    BF_UseProximityAccuracy,
    BF_CanUsePeekingVariations,
    BF_DisablePinnedDown,
    BF_d5265533,
    BF_2b84c2bf,
    BF_DisableBulletReactions,
    BF_CanBust,
    BF_aa525726,
    BF_CanCommandeerVehicles,
    BF_CanFlank,
    BF_56b08e10,
    BF_d30eb11d,
    BF_eb4786a0,
    BF_CanFightArmedPedsWhenNotArmed,
    BF_a08e9402,
    BF_952ead7d,
    BF_UseEnemyAccuracyScaling,
    BF_CanCharge,
    BF_da8c2bd3,
    BF_UseVehicleAttack,
    BF_CanThrowSignalsAttacking,
    BF_f49578b8,
    BF_CanSeeUnderwaterPeds,
    BF_f619486b,
    BF_61eb63a3,
    BF_195207ec,
    BF_8976d12b,
    BF_CanThrowSmokeGrenade,
    BF_NonMissionPedsFleeFromThisPedUnlessArmed,
    BF_5452a10c,
    BF_FleesFromInvincibleOpponents,
    BF_DisableBlockFromPursueDuringVehicleChase,
    BF_DisableSpinOutDuringVehicleChase,
    BF_DisableCruiseInFrontDuringBlockDuringVehicleChase,
    BF_b404731,
    BF_DisableReactToBuddyShot,
    BF_7ffd6aeb,
    BF_51f4aef8,
    BF_a5d0eab0,
    BF_63e0a8e2,
    BF_df974436,
    BF_556c080b,
    BF_a4d50035,
    BF_SetDisableShoutTargetPositionOnCombatStart,
    BF_DisableRespondedToThreatBroadcast,
    BF_cbb01765,
    BF_4f862ed4,
    BF_ef9c7c40,
    BF_e51b494f,
    BF_54d0199,
    BF_d36bce94,
    BF_fb11f690,
    BF_d208a9ad,
    BF_2806d078,
    BF_7a6e531,
    BF_34f9317b,
    BF_53aead9,
    BF_TOTAL_FLAGS
} eCombatBehaviourFlag;

class CPedIntelligence
{
public:
    GAME_ADDR_WRAPPER
    rage::bitset<BF_TOTAL_FLAGS> &
    GetCombatBehaviourFlags ()
    {
        return GameOffset<100089>::Get<rage::bitset<BF_TOTAL_FLAGS>> (this);
    }
};

class sMotionTaskData : public ParserWrapper<sMotionTaskData>
{
};

class CMotionTaskDataSet : public ParserWrapper<CMotionTaskDataSet>
{
};

class CMotionTaskDataManager
{
public:
    atArray<CMotionTaskDataSet *> aMotionTaskData;

    inline static GameFunction<100098, CMotionTaskDataSet*(uint32_t)> FindByName{};
};

struct CPedInventory
{
public:
    void *                   vft;
    uint8_t *                field_0x8;
    CPed *                   m_pPed;
    CInventoryItemRepository m_InventoryItemRepo;
    uint8_t                  field_0x3c[4];
    uint8_t                  field_0x40;
    uint8_t                  field_0x41[7];
    uint8_t *                field_0x48;
    uint32_t                 field_0x50;
    uint8_t                  field_0x54[4];
    CPedWeaponManager *      m_pManagers[2];
    uint32_t                 m_nNumManagers;
    uint8_t                  field_0x6c[4];
    uint8_t                  field_0x70;
    uint8_t                  field_0x71[7];
    uint8_t                  field_0x78;
    uint8_t                  field_0x79[7];

    GAME_ADDR_WRAPPER
    void *AddWeapon (uint32_t hash, uint32_t ammo)
    {
        return GameFunction<100095, void *(CPedInventory *, uint32_t,
                                           uint32_t)>::Call (this, hash, ammo);
    }
};

class CPed : public CEntity
{
public:
    inline static GameFunction<100096, CPed*(uint32_t)> GetFromHandle{};

    GAME_ADDR_WRAPPER
    CPedInventory *
    GetInventory ()
    {
        return GameOffset<100087>::Get<CPedInventory *> (this);
    }

    GAME_ADDR_WRAPPER
    inline CPedIntelligence *
    GetIntelligence ()
    {
        return GameOffset<100088>::Get<CPedIntelligence *> (this);
    }

    GAME_ADDR_WRAPPER
    aiTask *
    CreateMotionTask (sMotionTaskData *set, bool lowLod)
    {
        return GameFunction<100091, aiTask *(CPed *, sMotionTaskData *,
                                             bool)>::Call (this, set, lowLod);
    }

    GAME_ADDR_WRAPPER
    CVehicle *
    GetVehicle ()
    {
        return GameOffset<100092>::Get<CVehicle *> (this);
    }

    GAME_ADDR_WRAPPER
    uint32_t
    GetMotionState ()
    {
        return GameOffset<100093>::Get<uint32_t> (this);
    }

    GAME_ADDR_WRAPPER
    bool
    SetMotionState (uint32_t motionState, bool now)
    {
        return GameFunction<100094, bool (CPed *, uint32_t, bool)>::Call (
            this, motionState, now);
    }
};

class CPedFactory
{
public:
    void *vft;
    CPed *pPlayer;

    inline static GameVariable<CPedFactory*, 100090> sm_Instance{};

    static CPedFactory *
    Get ()
    {
        return sm_Instance;
    }
};
