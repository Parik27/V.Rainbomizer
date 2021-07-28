#pragma once

class phBoundComposite;
class CPedWeaponManager;
class CPed;
class aiTask;

/*******************************************************/
/* Animal Fixes - Ped Randomizer fixes required for animals to work properly and
 * not crash in most cases when a normal ped is randomized into an animal. Also
 * includes fixes to make them more convenient to use, such as preventing fish
 * to die, animals dying on ragdoll, etc.*/
/*******************************************************/
class PedRandomizer_AnimalFixes
{
    template <auto &phBoundComposite_CalculateExtents>
    static void
    AllocateTypeAndIncludeFlagsForFishBounds (phBoundComposite *bound, bool p1,
                                              bool p2);

    template <auto &CPedWeaponManager__ProcessFall>
    static void FixPoodleCrash (CPedWeaponManager *manager);

    template <auto &CPed__SetWanderTaskForPed>
    static aiTask *FixFallingPeds (CPed *_this, CPed *ped,
                                   bool *spawningPreference);

    void FixLadderClimbAsAnimal ();

public:
    static void Initialise ();
};
