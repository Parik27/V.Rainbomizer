#include "weapons_equipMgr.hh"
#include "scrThread.hh"
#include <algorithm>

/*******************************************************/
void
WeaponEquipMgr::AddWeaponToEquip (uint32_t weaponHash, uint32_t originalHash)
{
    if (!scrThread::GetActiveThread ())
        return;

    static uint8_t arrayCounter = 0;
    mRandomizedWeapons[(arrayCounter++) % 64]
        = {originalHash, weaponHash,
           scrThread::GetActiveThread ()->m_Context.m_nScriptHash};
}

/*******************************************************/
uint32_t
WeaponEquipMgr::GetWeaponToEquip (uint32_t weaponHash)
{
    if (!scrThread::GetActiveThread ())
        return weaponHash;

    uint32_t threadHash
        = scrThread::GetActiveThread ()->m_Context.m_nScriptHash;

    for (const auto &i : mRandomizedWeapons)
        if (i.originalHash == weaponHash && i.scrThread == threadHash)
            return i.weaponHash;

    return weaponHash;
}
