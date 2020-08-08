#include <Utils.hh>
#include <CPed.hh>

void *(*CPedInventory__GiveWeapon) (CPedInventory *, uint32_t, uint32_t);
CInventoryItem *(* CInventoryItemRepository__FindWeapon) (
    CInventoryItemRepository *, uint32_t);
CPed *(*fwScriptGuidPool__GetPed) (uint32_t handle);

/*******************************************************/
CInventoryItem *__cdecl
CInventoryItemRepository::FindWeapon (uint32_t hash)
{
    return CInventoryItemRepository__FindWeapon (this, hash);
}

/*******************************************************/
CPedInventory *
CPed::GetInventory ()
{
    static uint32_t nInvenOffset = *hook::get_pattern<uint32_t> (
        "8b ? ? ? ? ? ? 85 c9 74 ? ? 83 c1 18 8b d5", 2);

    return *(injector::raw_ptr (this) + nInvenOffset).get<CPedInventory*> ();
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
}
