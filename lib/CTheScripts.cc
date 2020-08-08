#include "CTheScripts.hh"
#include "Utils.hh"

/*******************************************************/
void
CTheScripts::InitialisePatterns ()
{
    aThreads = GetRelativeReference<atArray<scrThread *>> (
        "8b 0d ? ? ? ? 3b ca 7d ? ? 8b 0d ", 13, 17);
}

atArray<scrThread *>* CTheScripts::aThreads;
