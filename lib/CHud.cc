//?? 8d 0d ?? ?? ?? ?? ?? 0b c8 ?? 89 0c ??
//?? 39 84 ?? ?? ?? ?? ?? 75 ?? b9 15 00 00 00 

#include "CHud.hh"
#include "Utils.hh"

CARGB *CHud::aColours;
int *  CHud::aColourNameHashes;

/*******************************************************/
void
CHud::InitialisePatterns ()
{
    aColours
        = GetRelativeReference<CARGB> ("8d 0d ? ? ? ? ? 0b c8 ? 89 0c", 2, 6);

    aColourNameHashes
        = GetRelativeReference<int> ("39 84 ? ? ? ? ? 75 ? b9 15 00 00 00", 3);

    //*hook::get_pattern<int *> (
      //"39 84 ? ? ? ? ? 75 ? b9 15 00 00 00 ", 4);
}
