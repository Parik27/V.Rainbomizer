#include "CModelIndices.hh"
#include "Utils.hh"

void
CModelIndices::InitialisePatterns ()
{
    sm_Head = GetRelativeReference<CModelIndices *> (
        "40 53 ? 83 ec 20 ? 8b 1d ? ? ? ? eb ? 8b ? ", 9, 13);
}
