#include "CModelInfo.hh"
#include "CStreaming.hh"
#include "Utils.hh"
#include "Types.hh"
#include "scrThread.hh"

#include <cstdint>

#include <CPed.hh>
#include <CTask.hh>

#include <common/logger.hh>
#include <common/events.hh>

#include <peds/peds_Compatibility.hh>
#include <utility>

#include <CTheScripts.hh>

#ifdef ENABLE_DEBUG_MENU
#include <debug/motion.hh>
#endif

using namespace NativeLiterals;

class PedRandomizer_AnimalMotion
{
    inline static bool     bForceOriginalMotionNextTime = false;
    inline static uint32_t nPreviousPlayerMotionState   = 0;

    /*******************************************************/
    static auto *
    GetOnFootMotion (CMotionTaskDataSet *set)
    {
        return set->Get<sMotionTaskData *> ("onFoot"_joaat);
    }

    /*******************************************************/
    struct RandomMotionContext
    {
        CPed *              Ped;
        CPedModelInfo *     RandModel;
        CPedModelInfo *     OrigModel;
        sMotionTaskData *   OrigMotion;
        sMotionTaskData *   RandMotion;
        CMotionTaskDataSet *RandMotionSet;
        uint32_t            OrigMovementClipset;

        /*******************************************************/
        static CPedModelInfo *
        GetOriginalModel (CPed *ped)
        {
            auto *model
                = PedRandomizerCompatibility::GetCurrentPedRandomizing ();
            return model ? model
                         : PedRandomizerCompatibility::GetOriginalModel (ped);
        }

        /*******************************************************/
        uint32_t
        GetRandMotionType () const
        {
            return RandMotion->Equate ("Type"_joaat).ToHash ();
        }

        /*******************************************************/
        uint32_t
        GetOrigMotionType () const
        {
            return OrigMotion->Equate ("Type"_joaat).ToHash ();
        }

        /*******************************************************/
        bool
        AreOriginalAndRandomMotionSameType () const
        {
            return GetRandMotionType () == GetOrigMotionType ();
        }

        /*******************************************************/
        RandomMotionContext (CPed *ped, sMotionTaskData *data)
        {
            Ped        = ped;
            RandMotion = data;

            RandModel = static_cast<CPedModelInfo *> (ped->m_pModelInfo);
            OrigModel = GetOriginalModel (ped);

            OrigMotion = GetOnFootMotion (CMotionTaskDataManager::FindByName (
                OrigModel->GetInitInfo ().m_nMotionTaskDataSetName));

            RandMotionSet = CMotionTaskDataManager::FindByName (
                RandModel->GetInitInfo ().m_nMotionTaskDataSetName);

            OrigMovementClipset = RandModel->m_nMovementClipSet;
        }

        /*******************************************************/
        ~RandomMotionContext ()
        {
            RandModel->m_nMovementClipSet = OrigMovementClipset;
        }
    };

    /*******************************************************/
    static sMotionTaskData *
    OverrideWaterMotionForAnimals (const RandomMotionContext &ctx)
    {
        auto *landMotion = GetOnFootMotion (ctx.RandMotionSet);

        // ped not in water or ped's actually a ped
        if (landMotion == ctx.RandMotion
            || landMotion->Equate ("Type"_joaat) == "PED_ON_FOOT"_joaat)
            return nullptr;

        return landMotion;
    }

    /*******************************************************/
    static bool
    CanPedFly (CPed *ped)
    {
        auto *model   = static_cast<CPedModelInfo *> (ped->m_pModelInfo);
        auto *taskSet = CMotionTaskDataManager::FindByName (
            model->GetInitInfo ().m_nMotionTaskDataSetName);

        switch (GetOnFootMotion (taskSet)->Equate ("Type"_joaat).ToHash ())
            {
            case "FISH_IN_WATER"_joaat:
            case "BIRD_ON_FOOT"_joaat: return true;
            }

        return false;
    }

    /*******************************************************/
    static sMotionTaskData *
    MakeFishFly (const RandomMotionContext &ctx)
    {
        if (ctx.AreOriginalAndRandomMotionSameType ())
            return nullptr;

        if (ctx.GetRandMotionType () != "FISH_IN_WATER"_joaat)
            return nullptr;

        auto *seagullMotion
            = CMotionTaskDataManager::FindByName ("seagull"_joaat);

        auto *seagullModel
            = CStreaming::GetModelByHash<CPedModelInfo> ("a_c_seagull"_joaat);

        if (!seagullMotion || !seagullModel)
            return nullptr;

        ctx.RandModel->m_nMovementClipSet = seagullModel->m_nMovementClipSet;
        return GetOnFootMotion (seagullMotion);
    }

    /*******************************************************/
    static bool
    ShouldRestoreOriginalMotion (const RandomMotionContext &ctx)
    {
        if (ctx.AreOriginalAndRandomMotionSameType ())
            return false;

        if (std::exchange (bForceOriginalMotionNextTime, false))
            return true;

        switch (ctx.GetOrigMotionType ())
            {
                // Only for bird and fish
            case "BIRD_ON_FOOT"_joaat:
            case "FISH_IN_WATER"_joaat: return true;
            }

        // Fix engine starting by animals
        switch (ctx.Ped->GetMotionState ())
            {
            case "motionstate_invehicle"_joaat: return true;
            }

        return false;
    }

    /*******************************************************/
    static sMotionTaskData *
    RestoreOriginalMotion (const RandomMotionContext &ctx)
    {
        if (ShouldRestoreOriginalMotion (ctx))
            {
                ctx.RandModel->m_nMovementClipSet
                    = ctx.OrigModel->m_nMovementClipSet;

                return ctx.OrigMotion;
            }

        return nullptr;
    }

    /*******************************************************/
    static void
    FixAnimalSwimmingCrash (const RandomMotionContext &ctx)
    {
        auto *&inWater
            = ctx.RandMotionSet->Get<sMotionTaskData *> ("inWater"_joaat);
        if (!inWater)
            inWater = CMotionTaskDataManager::FindByName ("standard_ped"_joaat)
                          ->Get<sMotionTaskData *> ("inWater"_joaat);
    }

    /*******************************************************/
    template <auto &CPed__CreateMotionTask>
    static aiTask *
    ImproveAnimalMotion (CPed *ped, sMotionTaskData *data, bool lowLod)
    {
        RandomMotionContext ctx (ped, data);

        FixAnimalSwimmingCrash (ctx);
        if (ctx.Ped == CPedFactory::Get ()->pPlayer)
            {
                nPreviousPlayerMotionState = ctx.Ped->GetMotionState ();

#ifdef ENABLE_DEBUG_MENU
                MotionDebugInterface::SetLastKnownPlayerMotionState (
                    nPreviousPlayerMotionState);
#endif
            }

        if (auto newMotion = OverrideWaterMotionForAnimals (ctx))
            data = newMotion;

        if (auto newMotion = MakeFishFly (ctx))
            data = newMotion;

        if (auto newMotion = RestoreOriginalMotion (ctx))
            data = newMotion;

        return CPed__CreateMotionTask (ped, data, lowLod);
    }

    /*******************************************************/
    static bool
    IsPedAnAnimal (CPed *ped = CPedFactory::Get ()->pPlayer)
    {
        if (!ped)
            return false;

        auto *model = static_cast<CPedModelInfo *> (ped->m_pModelInfo);

        if (!model)
            return false;

        auto *motion = GetOnFootMotion (CMotionTaskDataManager::FindByName (
            model->GetInitInfo ().m_nMotionTaskDataSetName));

        if (!motion)
            return false;

        return !(motion->Equate ("Type"_joaat) == "PED_ON_FOOT"_joaat);
    }

    /*******************************************************/
    static bool
    HandleAnimalSwimming (SpecialBool &setInvisible)
    {
        static bool playerInWater = false;

        if (CanPedFly (CPedFactory::Get ()->pPlayer))
            return false;

        if (std::exchange (playerInWater,
                           "IS_PED_SWIMMING"_n("PLAYER_PED_ID"_n()))
            == playerInWater)
            return false;

        if (playerInWater)
            bForceOriginalMotionNextTime = true;

        setInvisible = playerInWater;
        return true;
    }

    /*******************************************************/
    static bool
    HandleAnimalShooting (SpecialBool &setInvisible)
    {
        static bool playerAiming = false;

        if (nPreviousPlayerMotionState == "motionstate_invehicle"_joaat)
            return false;

        if (std::exchange (playerAiming, "IS_AIM_CAM_ACTIVE"_n())
            == playerAiming)
            return false;

        setInvisible = playerAiming;

        if (playerAiming)
            bForceOriginalMotionNextTime = true;

        return true;
    }

    /*******************************************************/
    static void
    HandlePlayerAnimalMotion (uint64_t *stack, uint64_t *globals,
                              scrProgram *program, scrThreadContext *ctx)
    {
        if (ctx->m_nScriptHash != "main"_joaat)
            return;

        if (!IsPedAnAnimal ())
            return;

        SpecialBool setInvisible;

        if (HandleAnimalShooting (setInvisible)
            || HandleAnimalSwimming (setInvisible))
            {
                "SET_ENTITY_VISIBLE"_n("PLAYER_PED_ID"_n(), !setInvisible, 0);
                CPedFactory::Get ()->pPlayer->SetMotionState (
                    "motionstate_idle"_joaat, true);
            }
    }

    /*******************************************************/
    static void
    FixTakeCoverCheck (scrThread::Info *info)
    {
        CPed *ped = static_cast<CPed *> (
            fwScriptGuid::GetBaseFromGuid (info->GetArg (0)));

        if (scrThread::CheckActiveThread ("prologue1"_joaat)
            && IsPedAnAnimal (ped))
            {
                info->GetReturn () = true;
                return;
            }

        NativeManager::InvokeNative ("IS_PED_IN_COVER"_joaat, info);
    }

public:
    /*******************************************************/
    PedRandomizer_AnimalMotion ()
    {
        InitialiseAllComponents ();

        Rainbomizer::Events ().OnRunThread += HandlePlayerAnimalMotion;

        "IS_PED_IN_COVER"_n.Hook (FixTakeCoverCheck);

        REGISTER_HOOK ("? 8b 52 08 45 8a c1 e8 ? ? ? ? ? 8b c0 ? 8b c0 ", 7,
                       ImproveAnimalMotion, aiTask *, CPed *, sMotionTaskData *,
                       bool);
    }
} animlMo;
