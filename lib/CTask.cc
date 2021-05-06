#include "Patterns/Patterns.hh"
#include <CTask.hh>
#include <Utils.hh>

void (*aiTask__SetSubTask) (aiTask *, aiTask *);
void (*aiTask__DoSetState) (aiTask *, uint32_t);

/*******************************************************/
void
aiTask::DoSetState (uint32_t newState)
{
    aiTask__DoSetState (this, newState);
}

/*******************************************************/
void
aiTask::SetSubTask (aiTask *task)
{
    aiTask__SetSubTask (this, task);
}

/*******************************************************/
void
aiTask::InitialisePatterns ()
{
    ConvertCall (hook::get_pattern (
                     "? 89 5c ? ? 57 ? 83 ec 20 ? 8d 59 28 ? 8b fa ? 8b 0b"),
                 aiTask__SetSubTask);

    ConvertCall (hook::get_pattern ("40 53 ? 83 ec 20 0f be 41 36 ? 8b d9 3b "
                                    "d0 74 ? 66 83 49 30 40"),
                 aiTask__DoSetState);

    CTaskAimAndThrowProjectile::vftable = GetRelativeReference (
        "20 ? ? ? ? ? ? ? 8b ? ? 8b ? e8 ? ? ? ? ? 8d 05 ? ? "
        "? ? ? 8d ? b0 00 00 00 ? 8b d3 ? 89 ? e8",
        21, 25);

    CTaskShootAtTarget::vftable
        = GetRelativeReference ("? 8d 05 ? ? ? ? ? 8d 8e 90 00 00 00 ? 8b d3 ? "
                                "89 ? e8 ? ? ? ? ? 83 a6 ? 00 00 00 00",
                                3, 7);

    CTaskCombat::vftable = GetRelativeReference (
        "? 8d 05 ? ? ? ? ? 89 ? 66 89 ab b4 00 00 00 66 89 ab b5 00 00 00 ", 3,
        7);

    CTaskWeapon::vftable = GetRelativeReference (
        "? 8d 05 ? ? ? ? ? 8d 93 a0 00 00 00 ? 8d 8f a0 00 00 00 ? 89 ?", 3, 7);

    CTaskGun::vftable = GetRelativeReference (
        "? 8d 05 ? ? ? ? f3 0f 10 74 ? ? 0f 57 ff 0f 2f 35 ? ? ? ? ? 89 ? ", 3,
        7);
}
