#include "CVehicle.hh"
#include "Patterns/Patterns.hh"
#include <Utils.hh>
#include <CPed.hh>

void *(*CPedInventory__GiveWeapon) (CPedInventory *, uint32_t, uint32_t);
CInventoryItem *(*CInventoryItemRepository__FindWeapon) (
    CInventoryItemRepository *, uint32_t);
CPed *(*fwScriptGuidPool__GetPed) (uint32_t handle);
CMotionTaskDataSet *(*CMotionTaskDataManager__FindByName) (uint32_t name);
aiTask *(*CPed__CreateMotionTask) (CPed *, sMotionTaskData *, bool);

/*******************************************************/
CInventoryItem *__cdecl CInventoryItemRepository::FindWeapon (uint32_t hash)
{
    return CInventoryItemRepository__FindWeapon (this, hash);
}

/*******************************************************/
CPedInventory *
CPed::GetInventory ()
{
    static uint32_t nInvenOffset = *hook::get_pattern<uint32_t> (
        "8b ? ? ? ? ? ? 85 c9 74 ? ? 83 c1 18 8b d5", 2);

    return *(injector::raw_ptr (this) + nInvenOffset).get<CPedInventory *> ();
}

/*******************************************************/
CPedIntelligence *
CPed::GetIntelligence ()
{
    static uint32_t nIntelOffset
        = *hook::get_pattern<uint32_t> ("8b ? ? ? ? ? 0f ba ? ? ? ? ? 09 8b 05",
                                        2);

    return *(injector::raw_ptr (this) + nIntelOffset)
                .get<CPedIntelligence *> ();
}

/*******************************************************/
rage::bitset<BF_TOTAL_FLAGS> &
CPedIntelligence::GetCombatBehaviourFlags ()
{
    static uint32_t nBehaviourOffset
        = *hook::get_pattern<uint32_t> ("8b ? ? ? ? ? 0f ba ? ? ? ? ? 09 8b 05",
                                        9);

    return *(injector::raw_ptr (this) + nBehaviourOffset)
                .get<rage::bitset<BF_TOTAL_FLAGS>> ();
}

/*******************************************************/
CPed *
CPed::GetFromHandle (uint32_t handle)
{
    return fwScriptGuidPool__GetPed (handle);
}

/*******************************************************/
void *
CPedInventory::AddWeapon (uint32_t hash, uint32_t ammo)
{
    return CPedInventory__GiveWeapon (this, hash, ammo);
}

/*******************************************************/
CMotionTaskDataSet *
CMotionTaskDataManager::FindByName (uint32_t name)
{
    return CMotionTaskDataManager__FindByName (name);
}

/*******************************************************/
aiTask *
CPed::CreateMotionTask (sMotionTaskData *set, bool lowLod)
{
    return CPed__CreateMotionTask (this, set, lowLod);
}

/*******************************************************/
CVehicle *
CPed::GetVehicle ()
{
    static uint32_t nVehicleIndex = *hook::get_pattern<uint32_t> (
        "8b ? ? ? ? ? c1 e8 1e 41 84 c5 74 ? ? 83 ? ? ? ? ? 00 74 ? ba 0c 00 "
        "00 00 e9",
        17);

    return *GetAtOffset<CVehicle *> (this, nVehicleIndex);
}

/*******************************************************/
uint32_t
CPed::GetMotionState ()
{
    static uint32_t nMotionStateIndex = *hook::get_pattern<uint32_t> (
        "44 8b f2 ? 8b e9 e8 ? ? ? ? 33 ff 44 89 ? ? ? ? ?", 16);
    return *GetAtOffset<uint32_t> (this, nMotionStateIndex);
}

/*******************************************************/
void
CPedInventory::InitialisePatterns ()
{
    hook::pattern pattern (
        "57 ? 83 ec 20 ? 8b ? 8b ? ? 8b ? e8 ? ? ? ? ? 8b e8 ? 85 c0 74 ?");

    ConvertCall (pattern.get (1).get<void *> (-15), CPedInventory__GiveWeapon);

    ConvertCall (
        hook::get_pattern (
            "85 d2 74 ? 8b ca e8 ? ? ? ? ? 0f b7 47 08 ? 33 c9 ? ff c8", -15),
        CInventoryItemRepository__FindWeapon);

    ReadCall (hook::get_pattern (
                  "? 83 ec 20 ? 8b f8 8b ea e8 ? ? ? ? 33 db ? 85 c0", 9),
              fwScriptGuidPool__GetPed);

    ReadCall (
        hook::get_pattern ("? 89 ? ? ? ? ? 8b ? ? ? ? ? e8 ? ? ? ? ? 89 ? ? ? "
                           "? ? 8b ? ? ? ? ? e8 ? ? ? ? 33 d2 ? 8d ? ? ? ? ?",
                           13),
        CMotionTaskDataManager__FindByName);

    ReadCall (hook::get_pattern (
                  "? 8b 52 08 45 8a c1 e8 ? ? ? ? ? 8b c0 ? 8b c0 ", 7),
              CPed__CreateMotionTask);

    //?? 8b 05 ?? ?? ?? ?? ?? 8b d9 ?? 8b ?? ?? ?? 8b 42 20 ?? 85 c0
    CPedFactory::sm_Instance = GetRelativeReference<CPedFactory *> (
        "? 8b 05 ? ? ? ? ? 8b d9 ? 8b ? ? ? 8b 42 20 ? 85 c0", 3, 7);
}
