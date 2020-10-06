#include "CHandlingDataMgr.hh"
#include "Utils.hh"

void (*CHandlingData_InitValues) (CHandlingData*);

/*******************************************************/
void
CHandlingData::InitValues ()
{
    CHandlingData_InitValues (this);
}

/*******************************************************/
void
CHandlingDataMgr::InitialisePatterns ()
{
    sm_Instance = GetRelativeReference<CHandlingDataMgr> (
        "8b 0d ? ? ? ? 8b f3 ? 8a d5 ? 8b 0c ? e8", 2, 6);

    ReadCall (hook::get_pattern ("8b 0d ? ? ? ? 8b f3 ? 8a d5 ? 8b 0c ? e8",
                                 15),
              CHandlingData_InitValues);
}
