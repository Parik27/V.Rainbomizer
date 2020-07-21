#include <cstdint>
#include "CARGB.hh"

#pragma once

class CHud
{
public:
    // aColours[66]
    static CARGB *aColours;

    // aColourNameHashes
    static int *aColourNameHashes;

    static void InitialisePatterns ();
};
