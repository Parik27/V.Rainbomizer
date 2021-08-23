#include <common/config.hh>

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
                        strncpy (buff, "news.rainbomizer.com", outLen);
                    }
            }

        return buff;
    }

    /*******************************************************/
    template <auto &rage__formatf>
    static void
    MoveCloudCache (char *out, int len, char *format, char *, char *file)
    {
        static std::string s_CachePath
            = Rainbomizer::Common::GetRainbomizerFileName ("", "cloudcache/");
        rage__formatf (out, len, format, s_CachePath.c_str (), file);
    }

public:
    NewsRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("RainbomizerNews"))
            return;

        REGISTER_HOOK ("41 b8 80 00 00 00 ? 8b ce e8 ? ? ? ? ? 8b f8 ", 9,
                       SetRainbowDomain, char *, char *, char *, uint32_t);

        REGISTER_HOOK (
            "0f 44 ca 41 8b d0 ? 8d 05 ? ? ? ? e8 ? ? ? ? ? 83 c4 38 c3 ", 13,
            MoveCloudCache, void, char *, int, char *, const char *, char *);
    }
} news;
