#include <map>
#include <cstdint>

#include <Utils.hh>

#include <HSL.hh>
#include <CHud.hh>
#include <CARGB.hh>

#include "common/logger.hh"
#include "common/events.hh"
#include "common/config.hh"

void (*CHud__SetHudColour) (int, int, int, int, int);

class HudRandomizer
{
    inline static std::map<uint32_t, CARGB> mHudCols;

    /*******************************************************/
    static void
    SetNewHudColour (int index, int r, int g, int b, int a)
    {
        using Rainbomizer::HSL;

        mHudCols[index] = CARGB (a, r, g, b);

        HSL colour (CARGB (a, r, g, b));
        colour.h = RandomFloat (360);
        colour.s = RandomFloat (0.5, 1);
        colour.l = RandomFloat (std::max (0.0f, colour.l - 0.25f),
                                std::min (1.0f, colour.l + 0.25f));

        CARGB newColour = colour.ToARGB ();

        CHud__SetHudColour (index, newColour.r, newColour.g, newColour.b, a);
    }

    /*******************************************************/
    static void
    RandomizeOnFade ()
    {
        for (const auto &[idx, col] : mHudCols)
            SetNewHudColour (idx, col.r, col.g, col.b, col.a);
    }

public:
    /*******************************************************/
    HudRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("HudRandomizer"))
            return;

        InitialiseAllComponents ();

        // Hud Colours
        RegisterHook ("8b ? ? ? ? ? 8b ? ? ? ? ? 8b cb 89 44 ? ? e8", 18,
                      CHud__SetHudColour, SetNewHudColour);

        Rainbomizer::Events ().OnFade += RandomizeOnFade;
    }
} _cols;
