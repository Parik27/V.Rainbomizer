#include "Utils.hh" 
#include "common/logger.hh"

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
                        strncpy (buff, "local.parik.eu.org", outLen);
                    }
            }

        return buff;
    }

public:
    NewsRandomizer ()
    {
        REGISTER_HOOK ("41 b8 80 00 00 00 ? 8b ce e8 ? ? ? ? ? 8b f8 ", 9,
                       SetRainbowDomain, char *, char *, char *, uint32_t);
    }
} news;
