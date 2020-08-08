#pragma once

#include <cstdint>

class CText
{
public:
    static CText *TheText;

    char *GetText (uint32_t hash);
    bool HasThisAdditionalTextLoaded (const char* gxt, int slot);

    // sync means the game will wait for the pgStreamer to process the request
    // completely, i.e load the additional text synchronously and immediately.
    void RequestAdditionalText (uint32_t slot, const char *gxt, bool sync,
                                int dlc);

    void ClearAdditionalText (uint32_t slot, bool p2);
    
    static void InitialisePatterns ();
};
