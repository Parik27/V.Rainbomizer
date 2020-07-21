#pragma once

#include <cstdint>
#include "CEntity.hh"

class CInventoryItem;
class CPedWeaponManager;
class CPed;

struct CInventoryItemRepository
{
    CInventoryItem **  m_pInventoryItems;
    short              m_nNumInventoryItems;
    uint8_t            field_0xa[6];
    CPedWeaponManager *m_pManagers[2];
    int                m_nNumManagers;
};

struct CPedInventory
{
public:
    void *                   vft;
    uint8_t *                field_0x8;
    CPed *                   m_pPed;
    CInventoryItemRepository m_InventoryItemRepo;
    uint8_t                  field_0x3c[4];
    uint8_t                  field_0x40;
    uint8_t                  field_0x41[7];
    uint8_t *                field_0x48;
    uint32_t                 field_0x50;
    uint8_t                  field_0x54[4];
    CPedWeaponManager *      m_pManagers[2];
    uint32_t                 m_nNumManagers;
    uint8_t                  field_0x6c[4];
    uint8_t                  field_0x70;
    uint8_t                  field_0x71[7];
    uint8_t                  field_0x78;
    uint8_t                  field_0x79[7];

    void *AddWeapon (uint32_t hash, uint32_t ammo);

    static void InitialisePatterns ();
};

class CPed : public CEntity
{
public:
};
