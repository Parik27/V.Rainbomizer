#include <common/config.hh>

#include "Patterns/Patterns.hh"
#include "Utils.hh"

#include "common/logger.hh"
#include "common/common.hh"

#include <array>
#include <cstdint>
#include <cstdio>
#include <regex>
#include <string.h>
#include <utility>

class NewsRandomizer
{
    template <auto &rlCloud__GetSslServiceHostName>
    static char *
    SetRainbowDomain (char *prefix, char *out, uint32_t outLen)
    {
        char *buff = rlCloud__GetSslServiceHostName (prefix, out, outLen);

        if (!buff)
            {
                if (strstr (prefix, "/news/"))
                    {
                        buff = out;
                        strncpy (buff, "news.parik.eu.org/3.3", outLen);
                    }
            }

        return buff;
    }

public:
    NewsRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("RainbomizerNews"))
            return;

        char *cloudPath = hook::get_pattern<char> (
            "63 6c 6f 75 64 5f 25 30 31 36 49 36 34 78 2e 64 61 74 00");

        if (cloudPath)
            strcpy (cloudPath, "rbcld_%016I64x.dat");

        REGISTER_HOOK ("41 b8 80 00 00 00 ? 8b ce e8 ? ? ? ? ? 8b f8 ", 9,
                       SetRainbowDomain, char *, char *, char *, uint32_t);
    }
} news;
