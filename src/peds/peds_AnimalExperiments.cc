#include "CModelInfo.hh"
#include "CStreaming.hh"
#include "Utils.hh"

#include <cstdint>

#include <CPed.hh>
#include <CTask.hh>

#include <common/logger.hh>
#include <peds/peds_Compatibility.hh>
#include <utility>

class PedRandomizer_AnimalExperiments
{
    /*******************************************************/
    static auto *
    GetOnFootMotion (CMotionTaskDataSet *set)
    {
        return set->Get<sMotionTaskData *> ("onFoot"_joaat);
    }

    /*******************************************************/
    // Returns pair<set for original model, set for randomized model>
    static std::pair<CMotionTaskDataSet *, CMotionTaskDataSet *>
    GetPedMotionTaskDataSet (CPed *ped)
    {
        CPedModelInfo *rand = static_cast<CPedModelInfo *> (ped->m_pModelInfo);
        CPedModelInfo *orig
            = PedRandomizerCompatibility::GetOriginalModel (ped);

        return std::make_pair (
            CMotionTaskDataManager::FindByName (
                orig->GetInitInfo ().m_nMotionTaskDataSetName),
            CMotionTaskDataManager::FindByName (
                rand->GetInitInfo ().m_nMotionTaskDataSetName));
    }

    /*******************************************************/
    static aiTask *
    GetNewMotionSubtaskForPed (CPed *ped)
    {
        auto [origMS, newMS] = GetPedMotionTaskDataSet (ped);

        newMS->Get<sMotionTaskData *> ("inWater"_joaat)
            = CMotionTaskDataManager::FindByName ("standard_ped"_joaat)
                  ->Get<sMotionTaskData *> ("inWater"_joaat);
        ;

        // Check if the new motion set is actually a human locomotion
        // if (GetOnFootMotion (newMS)->Equate ("Type"_joaat)
        //     == GetOnFootMotion (origMS)->Equate ("Type"_joaat))
        //     return nullptr;

        return ped->CreateMotionTask (GetOnFootMotion (origMS), false);
    }

    /*******************************************************/
    template <auto &CTaskHumanLocomotion_UpdateFSM>
    static uint32_t
    ManageAnimalMotion (CTaskHumanLocomotion *task, uint32_t fsmState,
                        uint32_t fsmEvent)
    {
        if (!task->subTask && task->pPed == CPedFactory::Get ()->pPlayer)
            {
                if (auto subTask = GetNewMotionSubtaskForPed (task->pPed))
                    {
                        Rainbomizer::Logger::LogMessage (
                            "Created ped subtask thingy for ped: %p %p and "
                            "fsmState = %d, fsmEvent = %d",
                            task->pPed, task, fsmState, fsmEvent);
                        task->SetSubTask (subTask);
                        return 0;
                    }
            }
        else
            return 0;

        return CTaskHumanLocomotion_UpdateFSM (task, fsmState, fsmEvent);
    }

    /*******************************************************/
    template <auto &CTaskBirdLocomotion_UpdateFSM>
    static uint32_t
    ManageBirdLocomotion (CTaskBirdLocomotion *task, uint32_t fsmState,
                          uint32_t fsmEvent)
    {
        if (task->pPed->GetVehicle ())
            {
                task->DoSetState (100);
                return 0;
            }
        else if (task->pPed->GetMotionState () == "motionstate_aiming"_joaat)
            {
                task->DoSetState (101);
                return 0;
            }

        if (fsmState == 100)
            {
                if (fsmEvent == 0 || !task->subTask)
                    {
                        Rainbomizer::Logger::LogMessage (
                            "Creating task in motion vehicle");
                        task->SetSubTask (CTaskMotionInVehicle::Create ());
                    }
                else if (fsmEvent == 1)
                    {
                        if (!task->pPed->GetVehicle ())
                            {
                                task->DoSetState (0);
                            }
                    }

                return 0;
            }

        if (fsmState == 101)
            {
                if (fsmEvent == 0)
                    {
                        CTaskMotionAiming *subTask
                            = CTaskMotionAiming::Create ();

                        Rainbomizer::Logger::LogMessage (
                            "Creating aiming sub task");
                        task->SetSubTask (subTask);
                    }
                else if (fsmState == 1)
                    {
                        if (task->pPed->GetMotionState ()
                            != "motionstate_aiming"_joaat)
                            {
                                task->DoSetState (0);
                            }
                    }

                return 0;
            }

        return CTaskBirdLocomotion_UpdateFSM (task, fsmState, fsmEvent);
    }

    template <auto &CPed__CreateMotioNTask>
    static aiTask *
    ImproveAnimalMotion (CPed *ped, sMotionTaskData *data, bool lowLod)
    {
        auto *model = static_cast<CPedModelInfo *> (ped->m_pModelInfo);
        auto *origModel
            = PedRandomizerCompatibility::GetCurrentPedRandomizing ();

        if (!origModel)
            origModel = PedRandomizerCompatibility::GetOriginalModel (ped);

        auto *origMotionSet
            = GetOnFootMotion (CMotionTaskDataManager::FindByName (
                origModel->GetInitInfo ().m_nMotionTaskDataSetName));

        uint32_t origClipset = model->m_nMovementClipSet;

        if (data->Equate ("Type"_joaat) == "FISH_IN_WATER"_joaat)
            {
                auto *newMotion
                    = CMotionTaskDataManager::FindByName ("seagull"_joaat);

                auto *seagull = CStreaming::GetModelByHash<CPedModelInfo> (
                    "a_c_seagull"_joaat);

                if (newMotion && seagull)
                    {
                        data                      = GetOnFootMotion (newMotion);
                        model->m_nMovementClipSet = seagull->m_nMovementClipSet;
                    }
            }

        if ((origMotionSet->Equate ("Type"_joaat) == "BIRD_ON_FOOT"_joaat
             || origMotionSet->Equate ("Type"_joaat) == "FISH_IN_FOOT"_joaat)
            && origMotionSet->Get<uint32_t> ("Type"_joaat)
                   != data->Get<uint32_t> ("Type"_joaat))
            {
                data                      = origMotionSet;
                model->m_nMovementClipSet = origModel->m_nMovementClipSet;
            }

        aiTask *task = CPed__CreateMotioNTask (ped, data, lowLod);

        model->m_nMovementClipSet = origClipset;
        return task;
    }

public:
    PedRandomizer_AnimalExperiments ()
    {
        InitialiseAllComponents ();

        REGISTER_VFT_HOOK (CTaskHumanLocomotion, 14, ManageAnimalMotion,
                           uint64_t, CTaskHumanLocomotion *, uint32_t,
                           uint32_t);

        REGISTER_VFT_HOOK (CTaskBirdLocomotion, 14, ManageBirdLocomotion,
                           uint64_t, CTaskBirdLocomotion *, uint32_t, uint32_t);
    }
} /* animalExperiments */;
