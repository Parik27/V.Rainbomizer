#include "audEngine.hh"
#include "Utils.hh"

void *(*audMetadataManager_GetObjectByRef) (audMetadataMgr *, audMetadataRef);

void *
audMetadataMgr::FindObjectPtr (audMetadataRef ref)
{
    return audMetadataManager_GetObjectByRef (this, ref);
}

void
audEngine::InitialisePatterns ()
{
    audSpeechManager::sm_MetadataMgr = GetRelativeReference<audMetadataMgr> (
        "? 8d 2d ? ? ? ? ? 8b f0 85 d2 74 ? ", 3, 7);

    ConvertCall (hook::get_pattern (
                     "8d 42 01 ? 8b c9 a9 fe ff ff ff 75 ? 33 c0"),
                 audMetadataManager_GetObjectByRef);
}

audMetadataMgr *audSpeechManager::sm_MetadataMgr;
