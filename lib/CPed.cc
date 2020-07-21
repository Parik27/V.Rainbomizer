#include <Utils.hh>
#include <CPed.hh>

void* (*CPedInventory__GiveWeapon) (CPedInventory*, uint32_t, uint32_t);

/*******************************************************/
void*
CPedInventory::AddWeapon (uint32_t hash, uint32_t ammo)
{
  return CPedInventory__GiveWeapon(this, hash, ammo);
}

/*******************************************************/
void
CPedInventory::InitialisePatterns()
{
    hook::pattern pattern (
        "57 ? 83 ec 20 ? 8b ? 8b ? ? 8b ? e8 ? ? ? ? ? 8b e8 ? 85 c0 74 ?");

    ConvertCall (pattern.get (1).get<void *> (-15), CPedInventory__GiveWeapon);
}
