#include "CModelInfo.hh"
#include "CStreaming.hh"
#include "Utils.hh"

#include <cstdint>

#include <CPed.hh>
#include <CTask.hh>

#include <common/logger.hh>
#include <peds/peds_Compatibility.hh>
#include <utility>

class PedRandomizer_AnimalMotion
{
    /*******************************************************/
    static auto *
    GetOnFootMotion (CMotionTaskDataSet *set)
    {
        return set->Get<sMotionTaskData *> ("onFoot"_joaat);
    }

    /*******************************************************/
    struct RandomMotionContext
    {
        CPed *           Ped;
        CPedModelInfo *  RandModel;
        CPedModelInfo *  OrigModel;
        sMotionTaskData *OrigMotion;
        sMotionTaskData *RandMotion;
        uint32_t         OrigMovementClipset;

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
            RandModel  = static_cast<CPedModelInfo *> (ped->m_pModelInfo);
            OrigModel  = GetOriginalModel (ped);

            OrigMotion = GetOnFootMotion (CMotionTaskDataManager::FindByName (
                OrigModel->GetInitInfo ().m_nMotionTaskDataSetName));
            ;

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
    static sMotionTaskData *
    RestoreOriginalMotion (const RandomMotionContext &ctx)
    {
        if (ctx.AreOriginalAndRandomMotionSameType ())
            return nullptr;

        switch (ctx.GetOrigMotionType ())
            {
                // Only for bird and fish
            case "BIRD_ON_FOOT"_joaat:
            case "FISH_IN_WATER"_joaat:

                ctx.RandModel->m_nMovementClipSet
                    = ctx.OrigModel->m_nMovementClipSet;

                return ctx.OrigMotion;
            }

        return nullptr;
    }

    /*******************************************************/
    template <auto &CPed__CreateMotioNTask>
    static aiTask *
    ImproveAnimalMotion (CPed *ped, sMotionTaskData *data, bool lowLod)
    {
        RandomMotionContext ctx (ped, data);

        if (auto newMotion = MakeFishFly (ctx))
            data = newMotion;

        if (auto newMotion = RestoreOriginalMotion (ctx))
            data = newMotion;

        return CPed__CreateMotioNTask (ped, data, lowLod);
    }

public:
    PedRandomizer_AnimalMotion ()
    {
        InitialiseAllComponents ();

        REGISTER_HOOK ("? 8b 52 08 45 8a c1 e8 ? ? ? ? ? 8b c0 ? 8b c0 ", 7,
                       ImproveAnimalMotion, aiTask *, CPed *, sMotionTaskData *,
                       bool);
    }
} animlMo;
