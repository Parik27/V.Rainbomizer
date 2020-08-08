#include "audEngine.hh"
#include "Utils.hh"

void
audEngine::InitialisePatterns ()
{
    audSpeechManager::sm_MetadataMgr = GetRelativeReference<audMetadataMgr> (
        "? 8d 2d ? ? ? ? ? 8b f0 85 d2 74 ? ", 3, 7);
}

audMetadataMgr *audSpeechManager::sm_MetadataMgr;
