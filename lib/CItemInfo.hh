#pragma once

#include <cstdint>
#include "ParserUtils.hh"

#include <common/logger.hh>

class CItemInfo
{
public:
    uint8_t  field_0x8[8];
    uint32_t Name;
    uint32_t Model;
    uint32_t Audio;
    uint32_t Slot;

private:
    virtual void Destructor ();
    virtual bool GetIsClassId (uint32_t hash);

    // virtual uint32_t* GetClassId (); (older versions)
    virtual uint32_t *_GetClassId (uint32_t *out);

    // Not present in older versions of GTA V
    virtual uint32_t *_GetBaseClassId (uint32_t &out);

    // as a result, these functions are shifted by 1 function in the vftable.
    virtual uint32_t _GetModel ();

    virtual parStructure *_parser_GetStructure ();

public:
    //*******************************************************
    uint32_t
    GetClassId ()
    {
        uint32_t tmp;

        // Later versions return hash directly
        if (Rainbomizer::Logger::GetGameBuild () >= 2802)
            return static_cast<uint32_t> (uintptr_t (_GetClassId (nullptr)));
        else
            return *_GetClassId(&tmp);
    }
};

class CAmmoInfo : public ParserWrapper<CAmmoInfo>, public CItemInfo
{
};

class CWeaponInfo : public ParserWrapper<CWeaponInfo>, public CItemInfo
{
};

static_assert (CWeaponInfo::GetHash () == "CWeaponInfo"_joaat,
               "CWeaponInfo has wrong hash (compiler not supported?)");

class CWeaponInfoBlob : public ParserWrapper<CWeaponInfoBlob>
{
};

class CWeaponInfoManager
{
public:
    char                     field_0x0[16][4];
    atArray<CItemInfo *>     aItemInfos;
    uint8_t                  field_0x4c[4];
    atArray<CWeaponInfoBlob> aBlobs;
    uint8_t                  field_0x5c[36];
    uint32_t                 field_0x80;
    uint8_t                  field_0x84[4];
    uint64_t                 field_0x88;
    uint32_t                 field_0x90;

    static CWeaponInfoManager *sm_Instance;

    inline static CItemInfo *
    GetInfoFromIndex (int index)
    {
        return sm_Instance->aItemInfos[index];
    }

    static void InitialisePatterns ();
};
