#pragma once

#include <array>
#include <cstdint>

/*******************************************************/
/* Class to manage the equip-ing of the weapons randomized */
/*******************************************************/
class WeaponEquipMgr
{
    struct WeaponToEquip
    {
        uint32_t originalHash;
        uint32_t weaponHash;
        uint32_t scrThread;
    };
    
    std::array<WeaponToEquip, 64> mRandomizedWeapons;

public:

    WeaponEquipMgr () = default;
    WeaponEquipMgr (const WeaponEquipMgr& ) = delete;

    void     AddWeaponToEquip (uint32_t weaponHash, uint32_t originalHash);
    uint32_t GetWeaponToEquip (uint32_t weaponHash);
};
