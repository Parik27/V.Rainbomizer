#include <common/config.hh>

#include "Patterns/Patterns.hh"
#include "Utils.hh"

#include "common/logger.hh"
#include "common/common.hh"
#include "memory/GameAddress.hh"

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

        static GameVariable<char*, 100055> cloudPath{};

        if (cloudPath)
            strcpy (cloudPath, "rbcld_%016I64x.dat");

        REGISTER_HOOK (100056, SetRainbowDomain, char *, char *, char *, uint32_t);
    }
} news;
