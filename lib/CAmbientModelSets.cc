#include "CAmbientModelSets.hh"
#include "Utils.hh"

void
CAmbientModelSetsManager::InitialisePatterns ()
{
    sm_Instance = GetRelativeReference<CAmbientModelSetsManager *> (
        "? 8b 0d ? ? ? ? ? 8b c3 8b d0 e8 ? ? ? ?", 3, 7);
}
