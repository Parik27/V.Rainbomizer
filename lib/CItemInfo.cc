#include "CItemInfo.hh"
#include "Utils.hh"

void
CWeaponInfoManager::InitialisePatterns ()
{
    // 72 is the offset
    sm_Instance = reinterpret_cast<CWeaponInfoManager *> (
        GetRelativeReference<uint8_t> (
            "0f b7 15 ? ? ? ? ? 33 d2 2b d3 78 ? ? 8b 1d ? ? ? ?", 3,
            7)
        - 72);
}

CWeaponInfoManager *CWeaponInfoManager::sm_Instance = nullptr;
