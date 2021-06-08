#include "Utils.hh"
#include <CTask.hh>
#include <Utils.hh>
#include <common/logger.hh>

class TeachPedsHowToThrowProjectiles
{

    template <auto &CTaskShootAtTarget_UpdateFSM>
    static uint64_t
    TeachPedsProjectileThrowing (CTaskShootAtTarget *task, uint32_t fsmState,
                                 uint32_t fsmEvent)
    {

        if ((fsmState == 1 || fsmState == 2) && fsmEvent == 0)
            {
                auto *subTask
                    = CTaskAimAndThrowProjectile::Create (task->targetInfo);
                // subTask->nTaskId = eTaskType::CTaskGun;
                task->SetSubTask (subTask);

                return 0;
            }
        else
            {
                bool change = false;
                if (task->subTask
                    && task->nTaskId == eTaskType::CTaskAimAndThrowProjectile)
                    {
                        task->subTask->nTaskId = eTaskType::CTaskGun;
                        change                 = true;
                    }
                uint64_t ret
                    = CTaskShootAtTarget_UpdateFSM (task, fsmState, fsmEvent);

                if (change)
                    task->subTask->nTaskId
                        = eTaskType::CTaskAimAndThrowProjectile;

                return ret;
            }
    }

    template <auto &CTaskCombat_UpdateFSM>
    static uint64_t
    CombatHook (CTaskCombat *task, uint32_t fsmState, uint32_t fsmEvent)
    {
        if (fsmEvent == 0)
            Rainbomizer::Logger::LogMessage ("FSM State: %d", fsmState);
        if (fsmState == 2 && fsmEvent == 0)
            {
                task->DoSetState (33);
                return false;
            }

        return CTaskCombat_UpdateFSM (task, fsmState, fsmEvent);
    }

public:
    TeachPedsHowToThrowProjectiles ()
    {
        return;
        InitialiseAllComponents ();

        // REGISTER_VFT_HOOK (CTaskShootAtTarget, 14,
        // TeachPedsProjectileThrowing,
        //                     uint64_t, CTaskShootAtTarget*, uint32_t,
        //                     uint32_t);

        // REGISTER_VFT_HOOK (CTaskGun, 14, TeachPedsProjectileThrowingGun,
        //                    uint64_t, CTaskGun*, uint32_t, uint32_t);

        REGISTER_VFT_HOOK (CTaskCombat, 14, CombatHook, uint64_t, CTaskCombat *,
                           uint32_t, uint32_t);
    }
} _teachPedsHowToThrowProjectiles;
