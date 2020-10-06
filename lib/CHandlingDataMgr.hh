#pragma once

#include "rage.hh"
#include "ParserUtils.hh"

class CBaseSubHandlingData
{
public:
    virtual void          Destroy ();
    virtual parStructure *parser_GetStructure () const;
    virtual uint32_t      GetType ();
    virtual void          InitValues ();
};

#define DEFINE_SUB_HANDLING_CLASS(name)                                        \
    class name : public ParserWrapper<#name##_joaat>,                          \
                 public CBaseSubHandlingData                                   \
    {                                                                          \
    }

DEFINE_SUB_HANDLING_CLASS (CVehicleWeaponHandlingData);
DEFINE_SUB_HANDLING_CLASS (CSubmarineHandlingData);
DEFINE_SUB_HANDLING_CLASS (CBoatHandlingData);
DEFINE_SUB_HANDLING_CLASS (CCarHandlingData);
DEFINE_SUB_HANDLING_CLASS (CTrailerHandlingData);
DEFINE_SUB_HANDLING_CLASS (CSeaPlaneHandlingData);
DEFINE_SUB_HANDLING_CLASS (CFlyingHandlingData);
DEFINE_SUB_HANDLING_CLASS (CBikeHandlingData);
DEFINE_SUB_HANDLING_CLASS (CSpecialFlightHandlingData);

#undef DEFINE_SUB_HANDLING_CLASS

/*******************************************************/
class CHandlingData : public ParserWrapper<"CHandlingData"_joaat>
{
public:
    void InitValues ();
};

/*******************************************************/
class CHandlingDataMgr
{
public:
    inline static CHandlingDataMgr *sm_Instance = nullptr;

    atArray<CHandlingData *> pData;

    static void InitialisePatterns ();
};
