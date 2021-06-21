#include "CStreaming.hh"
#include "Utils.hh"

CBaseModelInfo *(*CStreaming__GetModelAndIndexByHash) (uint32_t  hash,
                                                       uint32_t *outIndex);
bool (*CStreaming__HasModelLoaded) (uint32_t *modelId);
void (*CStreaming__RequestModel) (uint32_t *modelId, uint32_t flags);
void (*CStreaming__LoadAllObjects) (bool priorityOnly);
void (*CStreaming__DeleteModel) (uint32_t *modelId);

/*******************************************************/
bool
CStreaming::HasModelLoaded (uint32_t modelId)
{
    return CStreaming__HasModelLoaded (&modelId);
}

/*******************************************************/
void
CStreaming::DeleteModel (uint32_t modelId)
{
    return CStreaming__DeleteModel (&modelId);
}

/*******************************************************/
void
CStreaming::RequestModel (uint32_t modelId, uint32_t flags)
{
    return CStreaming__RequestModel (&modelId, flags);
}

/*******************************************************/
void
CStreaming::LoadAllObjects (bool priorityOnly)
{
    return CStreaming__LoadAllObjects (priorityOnly);
}

/*******************************************************/
CBaseModelInfo *
CStreaming::GetModelAndIndexByHash (uint32_t hash, uint32_t &outIndex)
{
    return CStreaming__GetModelAndIndexByHash (hash, &outIndex);
}

/*******************************************************/
void
CStreaming::InitialisePatterns ()
{
    //? 8d ? 58 ? 23 c4 ? 23 c5 89 ? ? e8 ? ? ? ? ? 8d

    ReadCall (hook::get_pattern (
                  "? 8d ? 58 ? 23 c4 ? 23 c5 89 ? ? e8 ? ? ? ? ? 8d", 13),
              CStreaming__RequestModel);

    ReadCall (
        hook::get_pattern (
            "e8 ? ? ? ? 0f b7 ? ? 66 89 ? ? 8b ? ? ? 8b f8 81 c9 00 00 ff 0f "),
        CStreaming__GetModelAndIndexByHash);

    ReadCall (
        hook::get_pattern ("? 23 c4 ? 23 c5 89 ? ? e8 ? ? ? ? 84 c0 0f 84", 9),
        CStreaming__HasModelLoaded);

    ReadCall (hook::get_pattern ("41 b8 01 00 00 00 03 d3 e8 ? ? ? ? 33 c9 e8 "
                                 "? ? ? ? 8b 0d ? ? ? ? ",
                                 15),
              CStreaming__LoadAllObjects);

    ReadCall (hook::get_pattern ("? 23 c4 89 44 ? ? e8 ? ? ? ? ff cb ", 7),
              CStreaming__DeleteModel);

    ms_aModelPointers = GetRelativeReference<modelInfoArray> (
        "33 d2 ? 8b d0 ? 2b 05 ? ? ? ? c1 e6 10", 8, 12);

    sm_Instance = GetRelativeReference<CStreaming> (
        " 8b 54 ? ? ? 8d 0d ? ? ? ? e8 ? ? ? ? 8a c3 ", 7, 11);
}

modelInfoArray *CStreaming::ms_aModelPointers;
CStreaming *    CStreaming::sm_Instance;
