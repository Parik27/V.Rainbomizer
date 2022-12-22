#include "audEngine.hh"
#include "Utils.hh"

void *(*audMetadataManager_GetObjectByRef) (audMetadataMgr *, audMetadataRef);
void *(*audSound_DecompressMetadata_Untyped) (rage::Sound *, rage::Sound *);

void *
audMetadataMgr::FindObjectPtr (audMetadataRef ref)
{
    return audMetadataManager_GetObjectByRef (this, ref);
}

void *
audSound::DecompressMetadata_Untyped (rage::Sound *inp, rage::Sound *out)
{
    return audSound_DecompressMetadata_Untyped (inp, out);
}

uint32_t
audMetadataChunk::GetSize ()
{
    static uint32_t size
        = *hook::get_pattern<uint8_t> ("8b 44 ? ? ? 6b d2 ? ? 03 44 0a 10", 7);

    return size;
}

void
audEngine::InitialisePatterns ()
{
    audSpeechManager::sm_MetadataMgr = GetRelativeReference<audMetadataMgr> (
        "? 8d 2d ? ? ? ? ? 8b f0 85 d2 74 ? ", 3, 7);

    ConvertCall (hook::get_pattern (
                     "8d ? ? ? 8b ? a9 fe ff ff ff 75 ? 33 ? c3 "),
                 audMetadataManager_GetObjectByRef);

    ConvertCall (hook::get_pattern (
                     "33 c0 ? 8b c9 ? 8d 41 05 88 42 3c 66 89 42 09"),
                 audSound_DecompressMetadata_Untyped);
}

audMetadataMgr *audSpeechManager::sm_MetadataMgr;
