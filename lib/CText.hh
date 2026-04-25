#pragma once

#include "memory/GameAddress.hh"
#include <cstdint>

class CText
{
public:
    inline static GameVariable<CText, 100123> TheText{};
    inline static GameVariable<uint32_t, 100124> LabelFoundAt{};

    char *GetText (uint32_t hash)
    {
        return GameFunction<100122, char* (CText*, uint32_t)>::Call (this, hash);
    }

    bool  HasThisAdditionalTextLoaded (const char *gxt, int slot)
    {
        return GameFunction<100125, bool (CText*, const char*, int)>::Call (this, gxt, slot);
    }

    // sync means the game will wait for the pgStreamer to process the request
    // completely, i.e load the additional text synchronously and immediately.
    void RequestAdditionalText (uint32_t slot, const char *gxt, bool sync,
                                int dlc)
    {
        return GameFunction<100126, void (CText*, uint32_t, const char*, bool, int)>::Call
            (this, slot, gxt, sync, dlc);
    }
};
