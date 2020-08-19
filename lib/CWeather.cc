#include "CWeather.hh"
#include <Utils.hh>

/*******************************************************/
void
CWeather::InitialisePatterns ()
{
    g_Weather = GetRelativeReference<CWeather> (
        "69 c0 c4 00 00 00 ? 03 ? eb ? ? 8d 0d ? ? ? ? e8", 14, 18);
}
