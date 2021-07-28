#include "peds_AnimalFixes.hh"
#include "CStreaming.hh"
#include "Patterns/Patterns.hh"
#include "injector/injector.hpp"
#include "phBound.hh"
#include <Utils.hh>
#include <CPed.hh>
#include <cstdint>
#include <CTask.hh>
#include "peds_Compatibility.hh"

/*******************************************************/
template <auto &CPedWeaponManager__ProcessFall>
void
PedRandomizer_AnimalFixes::FixPoodleCrash (CPedWeaponManager *manager)
{
#define PED_RANDOMIZER_INCLUDE_ALL_PEDS_IN_POODLE_FIX
#ifdef PED_RANDOMIZER_INCLUDE_ALL_PEDS_IN_POODLE_FIX
    return;
#endif

    if (manager && manager->m_pPed && manager->m_pPed->m_pModelInfo)
        {
            uint32_t hash = manager->m_pPed->m_pModelInfo->m_nHash;
            switch (hash)
                {
                case "a_c_boar"_joaat:
                case "a_c_cat_01"_joaat:
                case "a_c_chickenhawk"_joaat:
                case "a_c_chimp"_joaat:
                case "a_c_chop"_joaat:
                case "a_c_cormorant"_joaat:
                case "a_c_cow"_joaat:
                case "a_c_coyote"_joaat:
                case "a_c_crow"_joaat:
                case "a_c_deer"_joaat:
                case "a_c_dolphin"_joaat:
                case "a_c_fish"_joaat:
                case "a_c_hen"_joaat:
                case "a_c_humpback"_joaat:
                case "a_c_husky"_joaat:
                case "a_c_killerwhale"_joaat:
                case "a_c_mtlion"_joaat:
                case "a_c_pig"_joaat:
                case "a_c_pigeon"_joaat:
                case "a_c_poodle"_joaat:
                case "a_c_pug"_joaat:
                case "a_c_rabbit_01"_joaat:
                case "a_c_rat"_joaat:
                case "a_c_retriever"_joaat:
                case "a_c_rhesus"_joaat:
                case "a_c_rottweiler"_joaat:
                case "a_c_seagull"_joaat:
                case "a_c_sharkhammer"_joaat:
                case "a_c_sharktiger"_joaat:
                case "a_c_shepherd"_joaat:
                case "a_c_stingray"_joaat:
                case "a_c_westy"_joaat: return;
                }
        }

    CPedWeaponManager__ProcessFall (manager);
}

/*******************************************************/
template <auto &phBoundComposite_CalculateExtents>
void
PedRandomizer_AnimalFixes::AllocateTypeAndIncludeFlagsForFishBounds (
    phBoundComposite *bound, bool p1, bool p2)
{
    if (!bound->OwnedTypeAndIncludeFlags)
        {
            bound->AllocateTypeAndIncludeFlags ();
            bound->TypeAndIncludeFlags[1] &= 3221225471;
        }

    phBoundComposite_CalculateExtents (bound, p1, p2);
}

/*******************************************************/
template <auto &CPed__SetWanderTaskForPed>
aiTask *
PedRandomizer_AnimalFixes::FixFallingPeds (CPed *_this, CPed *ped,
                                           bool *spawningPreference)
{
    const int DSP_AERIAL = 1;
    if (spawningPreference[DSP_AERIAL])
        {
            // This makes it so that the animal motion component thinks that
            // the original ped was pigeon so the proper animations could be
            // assigned to it.

            PedRandomizer_Compatibility::AddRandomizedPed (
                _this, CStreaming::GetModelIndex ("a_c_pigeon"_joaat),
                CStreaming::GetModelIndex (ped->m_pModelInfo->m_nHash));

            ped->SetMotionState ("motionstate_idle"_joaat, true);
        }

    return CPed__SetWanderTaskForPed (_this, ped, spawningPreference);
}

/*******************************************************/
void
PedRandomizer_AnimalFixes::FixLadderClimbAsAnimal ()
{
    uint8_t xmm0_0p57[] = {
        0xb8, 0x85, 0xeb, 0x11, 0x3f, // mov    eax,0x3f11eb85
        0x66, 0x0f, 0x6e, 0xc0        // movd   xmm0,eax
    };

    void *ptr = hook::get_pattern (
        "f3 0f 10 40 64 8a 42 0c fe c8 3c 01 0f 96 ? f6 d9 ? 1b c0 ? 23 c2 "
        "f3 0f 5c 40 40 f3 0f 58 f8 ");
    injector::MakeNOP (ptr, 28);
    injector::WriteMemoryRaw (ptr, xmm0_0p57, sizeof (xmm0_0p57), true);
}

/*******************************************************/
void
PedRandomizer_AnimalFixes::Initialise ()
{
    // Hook to prevent peds with flag (DiesOnRagdoll) from dying
    injector::MakeNOP (
        hook::get_pattern ("75 ? ? 8b ? ? ? ? ? ? 85 c0 74 ? 44 38 60 0f 75",
                           18),
        2);

    // Birds don't die immediately on hitting anything
    injector::WriteMemory<uint16_t> (
        hook::get_pattern (
            "? 83 ? ? ? ? ? 00 0f 84 ? ? ? ? ? 8b cf e8 ? ? ? ? 8a 4f 36", 8),
        0xe948);

    // This hook fixes a crash in CTaskUseScenario, where the ped bounds
    // type and include flags are accessed; In case of fish bounds, these
    // type and include flags are not allocated causing a crash.
    REGISTER_HOOK ("0f 29 18 0f 29 50 10 0f 29 48 20 0f 29 40 30 e8", 15,
                   AllocateTypeAndIncludeFlagsForFishBounds, void,
                   phBoundComposite *, bool, bool);

    REGISTER_HOOK ("? 83 c6 08 3b f1 7c ? ? 8b ? ? ? ? ? ? 85 c9 74 ? e8 ? "
                   "? ? ? ? 8b ? ? ? ? ? ? 8b 03 ",
                   20, FixPoodleCrash, void, CPedWeaponManager *);

    REGISTER_HOOK ("? 8b cf ? 8b c7 e8 ? ? ? ? ? 8b d0 ? 85 c0 74 ?", 6,
                   FixFallingPeds, aiTask *, CPed *, CPed *, bool *);

    // This patch fixes CTaskExitVehicle from crashing the game if the ped
    // exiting the vehicle doesn't have a helmet.
    injector::MakeNOP (
        hook::get_pattern ("44 38 70 0a 74 ? 40 84 f6 75 ? b9 ? ? ? ? e8", 4),
        2);

    FixLadderClimbAsAnimal ();
}
