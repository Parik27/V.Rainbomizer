#pragma once

#include <Utils.hh>
#include <array>

#include "CModelInfo.hh"
#include "ParserUtils.hh"
#include "mission/missions_YscUtils.hh"
#include "peds_Compatibility.hh"

#include <CPed.hh>
#include <CutSceneManager.hh>
#include <CModelIndices.hh>
#include <cstdint>
#include <rage.hh>
#include <utility>

#include <CTheScripts.hh>
#include <CStreaming.hh>

#include "common/ysc.hh"

struct fwRefAwareBase;

class CPlayerPedSaveStructure : public ParserWrapper<CPlayerPedSaveStructure>
{
};

class PedRandomizer_PlayerFixes
{
    inline static bool                           sm_Initialised = false;
    inline static std::array<CModelIndices *, 3> sm_PlayerIdxPtr;
    inline static std::array<uint32_t, 3>        sm_OrigPlayerAbility;

    /*******************************************************/
    static int
    GetRandomSpecialAbility (bool unused)
    {
        if (!unused)
            return GetRandomElement (sm_OrigPlayerAbility);

        static unsigned int numSpecialAbilities = 0;
        if (numSpecialAbilities == 0)
            {
                static struct parEnumTranslationMap
                {
                    uint32_t hash;
                    int32_t  value;
                } *enumTranslationTable
                    = hook::get_pattern<parEnumTranslationMap> (
                        "c0 24 e0 31 ff ff ff ff 64 d0 6a 7f 00 00 00 00");

                while (enumTranslationTable[++numSpecialAbilities].hash != 0)
                    ;
            }

        return RandomInt (numSpecialAbilities - 2);
    }

    /*******************************************************/
    template <auto &CutSceneManager_InitPlayerObj>
    static void
    CorrectPlayerObjIdx (CutSceneManager *mgr)
    {
        CPed *          player = CPedFactory::Get ()->pPlayer;
        CBaseModelInfo *model  = PedRandomizerCompatibility::GetOriginalModel (
            CPedFactory::Get ()->pPlayer);

        // Restore original model for this function and change it back after
        // done.
        std::swap (player->m_pModelInfo, model);
        CutSceneManager_InitPlayerObj (mgr);
        std::swap (player->m_pModelInfo, model);
    }

    /*******************************************************/
    static CModelIndices *
    GetPlayerModelIndexPtr (uint32_t hash)
    {
        switch (hash)
            {
            case "player_zero"_joaat: return sm_PlayerIdxPtr[0];
            case "player_one"_joaat: return sm_PlayerIdxPtr[1];
            case "player_two"_joaat: return sm_PlayerIdxPtr[2];
            }

        return nullptr;
    }

    /*******************************************************/
    static void
    InitialisePlayerIndicesPointers ()
    {
        static bool bPlayerIndicesIntiailised = false;
        if (std::exchange (bPlayerIndicesIntiailised, true))
            return;

        for (auto index = CModelIndices::GetHead (); index;
             index      = index->pNext)
            {
                switch (index->nHash)
                    {
                    case "player_zero"_joaat: sm_PlayerIdxPtr[0] = index; break;
                    case "player_one"_joaat: sm_PlayerIdxPtr[1] = index; break;
                    case "player_two"_joaat: sm_PlayerIdxPtr[2] = index; break;
                    }
            }
    }

    /*******************************************************/
    static void
    ResetPlayerHashChanges ()
    {
        sm_PlayerIdxPtr[0]->nHash = "player_zero"_joaat;
        sm_PlayerIdxPtr[1]->nHash = "player_one"_joaat;
        sm_PlayerIdxPtr[2]->nHash = "player_two"_joaat;
    }

    /*******************************************************/
    template <auto &fwRefAwareBase__AddKnownRef>
    static void
    UpdatePlayerHash_Trampoline (fwRefAwareBase *base, void *ref)
    {
        fwRefAwareBase__AddKnownRef (base, ref);
        UpdatePlayerHash ();
    }

    /*******************************************************/
    static bool
    RemoveLester1ClothesRequirement (YscUtilsOps &ops)
    {
        if (!ops.IsAnyOf ("lester1"_joaat))
            return false;

        ops.Init ("38 0e 25 ? 08 2a 06 56 ? ?");
        ops.NOP (/*Offset=*/7, /*Size=*/3);

        ops.Init ("2a 06 56 ? ? 38 0d 25 48 08 20");
        ops.NOP (/*Offset=*/2, /*Size=*/3);

        return true;
    }

    /*******************************************************/
    static bool
    RemoveJewelrySetupClothesRequirement (YscUtilsOps &ops)
    {
        if (!ops.IsAnyOf ("mission_triggerer_a"_joaat))
            return false;

        uint8_t return_0x1[] = {0x6f, 0x2e, 0x00, 0x01};

        ops.Init ("2d 00 03 00 ? 2c ? 00 74 71 5d ? ? ? 39 02 38 02 6e");
        ops.WriteBytes (/*Offset=*/5, return_0x1);

        return true;
    }

    /*******************************************************/
    static bool
    RemoveMichael4ClothesRequirement (YscUtilsOps &ops)
    {
        if (!ops.IsAnyOf ("michael4"_joaat))
            return false;

        uint8_t return_0x1[] = {0x6f, 0x2e, 0x03, 0x01};

        ops.Init ("26 0c 10 5d ? ? ? 06 56 ? ? 28 c9 14 71 0d");
        ops.FollowBranchDestination (/*Offset=*/3);
        ops.WriteBytes (/*Offset=*/5, return_0x1);

        return true;
    }

    /*******************************************************/
    template <auto &CPlayerPedSaveStructure__PreSave>
    static void
    FixSavedPlayerModel (CPlayerPedSaveStructure *save)
    {
        CPlayerPedSaveStructure__PreSave (save);

        uint32_t origHash = PedRandomizerCompatibility::GetOriginalModel (
                                CPedFactory::Get ()->pPlayer)
                                ->m_nHash;

        save->Get<uint32_t> ("ModelHashKey"_joaat) = origHash;
    }

public:
    /*******************************************************/
    static void
    Initialise ()
    {
        if (sm_Initialised)
            return;

        sm_Initialised = true;

        // Hook for cutscenes to properly get the object model to register for
        // the cutscene.
        REGISTER_HOOK ("8b d9 40 38 3d ? ? ? ? 75 ? e8", 11,
                       CorrectPlayerObjIdx, void, CutSceneManager *);

        // Hook to reset the saved player model during saving to make sure
        // you don't get a random player on loading without Rainbomizer and
        // (more importantly) to prevent softlocks and crashes this causes.
        // Pattern from Robot (don't insert u word here)
        REGISTER_JMP_HOOK (12,
                           "? 89 5c ? ? 55 56 57 41 54 41 55 41 56 41 57 ? 8d "
                           "? ? d9 ? 81 ec 90 00 00 00 ? 8b ? ? ? ? ? 33 f6 ? "
                           "8b f1 ? 8b ? ? ? 85 ff 0f 84 ? ? ? ? ? 8b",
                           0, FixSavedPlayerModel, void,
                           CPlayerPedSaveStructure *);

        // This patch enables phone models for all ped models
        injector::MakeNOP (
            hook::get_pattern (
                "8b ? ? ? ? ? 44 38 70 09 74 ? 40 84 f6 75 ? b9 30 00 00 00 ",
                10),
            2);

        // To make UpdatePlayerHash also be called after a call to
        // CHANGE_PLAYER_PED (and several other functions). Can also be extended
        // to add CPedFactory::CreatePlayer, but I didn't see the need.
        // UpdatePlayerHash is also called by Ped Randomizer to catch those
        // cases.
        REGISTER_HOOK (
            "? 8d ? 08 ? 8b cf ? 89 ? ? e8 ? ? ? ? ? 8d 0d ? ? ? ? ? 8b d7", 11,
            UpdatePlayerHash_Trampoline, void, fwRefAwareBase *, void *);

        YscCodeEdits::Add ("Remove Lester1 Clothes Requirement",
                           RemoveLester1ClothesRequirement);
        YscCodeEdits::Add ("Remove JewelrySetup Clothes Requirement",
                           RemoveJewelrySetupClothesRequirement);
        YscCodeEdits::Add ("Remove Michael4 Clothes Requirement",
                           RemoveMichael4ClothesRequirement);

#define HOOK(native, func) NativeCallbackMgr::Add<native##_joaat, func> ()
        HOOK ("SPECIAL_ABILITY_UNLOCK", FixSpecialAbilityLock);
        HOOK ("SPECIAL_ABILITY_LOCK", FixSpecialAbilityLock);
#undef HOOK
    }

    /*******************************************************/
    static void
    InitSpecialAbilityValues ()
    {
        static bool bInitialised = false;
        if (std::exchange (bInitialised, true))
            return;

        InitialisePlayerIndicesPointers ();

        int i = 0;
        for (auto hash :
             {"player_zero"_joaat, "player_one"_joaat, "player_two"_joaat})
            {
                auto *model = CStreaming::GetModelByIndex<CPedModelInfo> (
                    GetPlayerModelIndexPtr (hash)->nIndex);

                sm_OrigPlayerAbility[i++]
                    = model->GetInitInfo ().m_nSpecialAbility;
            }
    }

    /*******************************************************/
    static unsigned int
    GetOrigSpecialAbility (CPed *ped)
    {
        InitSpecialAbilityValues ();
        auto *model = PedRandomizerCompatibility::GetOriginalModel (ped);
        if (!model)
            return 0;

        switch (model->m_nHash)
            {
            case "player_zero"_joaat: return sm_OrigPlayerAbility[0];
            case "player_one"_joaat: return sm_OrigPlayerAbility[1];
            case "player_two"_joaat: return sm_OrigPlayerAbility[2];
            }

        return 0;
    }

    /*******************************************************/
    static void
    SetSpecialAbility (CPed *ped, bool randomize, bool unused)
    {
        InitSpecialAbilityValues ();
        auto model = static_cast<CPedModelInfo *> (ped->m_pModelInfo);
        if (randomize)
            model->GetInitInfo ().m_nSpecialAbility
                = GetRandomSpecialAbility (unused);
        else
            model->GetInitInfo ().m_nSpecialAbility
                = GetOrigSpecialAbility (ped);
    }

    /*******************************************************/
    static void
    FixSpecialAbilityLock (scrThread::Info *info, bool before)
    {
        if (before)
            {
                InitialisePlayerIndicesPointers ();
                ResetPlayerHashChanges ();
            }
        else
            UpdatePlayerHash ();
    }

    /*******************************************************/
    static void
    UpdatePlayerHash ()
    {
        InitialisePlayerIndicesPointers ();
        ResetPlayerHashChanges ();

        auto *player = CPedFactory::Get ()->pPlayer;
        if (!player)
            return;

        auto *model = PedRandomizerCompatibility::GetOriginalModel (player);
        if (!model)
            return;

        if (auto idxPtr = GetPlayerModelIndexPtr (model->m_nHash))
            idxPtr->nHash = player->m_pModelInfo->m_nHash;
    }
};
