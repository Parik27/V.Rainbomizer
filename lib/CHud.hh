#include <cstdint>
#include "CARGB.hh"
#include "memory/GameAddress.hh"

#pragma once

class CHud
{
public:
    // aColours[66]
    inline static GameVariable<CARGB*, 100099> aColours;

    // aColourNameHashes
    inline static GameVariable<int*, 100100> aColourNameHashes;
};
