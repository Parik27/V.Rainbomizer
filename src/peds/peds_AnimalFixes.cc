#include "phBound.hh"
#include <Utils.hh>

/*******************************************************/
/* Animal Fixes - Ped Randomizer fixes required for animals to work properly and
 * not crash in most cases when a normal ped is randomized into an animal. Also
 * includes fixes to make them more convenient to use, such as preventing fish
 * to die, animals dying on ragdoll, etc.*/
/*******************************************************/
class PedRandomizer_AnimalFixes
{
    /*******************************************************/
    template <auto &phBoundComposite_CalculateExtents>
    static void
    AllocateTypeAndIncludeFlagsForFishBounds (phBoundComposite *bound, bool p1,
                                              bool p2)
    {
        if (!bound->OwnedTypeAndIncludeFlags)
            {
                bound->AllocateTypeAndIncludeFlags ();
                bound->TypeAndIncludeFlags[1] &= 3221225471;
            }

        phBoundComposite_CalculateExtents (bound, p1, p2);
    }

public:
    /*******************************************************/
    PedRandomizer_AnimalFixes ()
    {
        // Hook to prevent peds with flag (DiesOnRagdoll) from dying
        // injector::MakeNOP (
        //     hook::get_pattern (
        //         "75 ? ? 8b ? ? ? ? ? ? 85 c0 74 ? 44 38 60 0f 75", 18),
        //     2);

        // This hook fixes a crash in CTaskUseScenario, where the ped bounds
        // type and include flags are accessed; In case of fish bounds, these
        // type and include flags are not allocated causing a crash.
        REGISTER_HOOK ("0f 29 40 30 e8 ? ? ? ? b2 01 ? 8b cf e8", 4,
                       AllocateTypeAndIncludeFlagsForFishBounds, void,
                       phBoundComposite *, bool, bool);

        // This patch fixes CTaskExitVehicle from crashing the game if the ped
        // exiting the vehicle doesn't have a helmet.
        injector::MakeNOP (
            hook::get_pattern ("44 38 70 0a 74 ? 40 84 f6 75 ? b9 ? ? ? ? e8",
                               4),
            2);
    }
} peds_AnimalFixes;
