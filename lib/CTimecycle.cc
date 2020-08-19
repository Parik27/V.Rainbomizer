#include "CTimecycle.hh"
#include "Utils.hh"

void
tcManager::InitialisePatterns ()
{
    tcConfig::ms_numVars = GetRelativeReference<uint32_t> (
        "89 15 ? ? ? ? ? 89 05 ? ? ? ? ? 8b 49 58 e8", 2, 6);

    tcConfig::ms_pVarInfos = GetRelativeReference<tcVarInfo *> (
        "89 15 ? ? ? ? ? 89 05 ? ? ? ? ? 8b 49 58 e8", 9, 13);

    tcConfig::ms_cycleInfo = GetRelativeReference<atArray<char[64]>> (
        "89 05 ? ? ? ? 66 89 35 ? ? ? ? 85 f6 7e ? ? 8b c6 ", 2, 6);

    g_timeCycle = GetRelativeReference<tcManager> (
        "f3 0f 10 72 78 ? 8d 0d ? ? ? ? 33 d2 e8", 8, 12);
}
