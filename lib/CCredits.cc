#include <CCredits.hh>
#include <Utils.hh>

void
CCreditArray::InitialisePatterns ()
{
    sm_Instance = GetRelativeReference<CCreditArray> (
        "? 8b 05 ? ? ? ? 83 3c 03 0e e9 ? ? ? ? 66", 3, 7);
}
