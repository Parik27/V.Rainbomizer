#include <CHud.hh>
#include <Utils.hh>
#include "common/config.hh"
#include "HSL.hh"
#include <CVehicle.hh>

void (*CHud__SetHudColour) (int, int, int, int, int);
uint32_t (*CCustomShaderEffectVehicle_SetForVehicle_134) (
    CCustomShaderEffectVehicle *, CVehicle *);

class ColoursRandomizer
{
    /*******************************************************/
    static void
    SetNewHudColour (int index, int r, int g, int b, int a)
    {
        using Rainbomizer::HSL;

        HSL colour (CARGB (a, r, g, b));
        colour.h = RandomFloat (360);
        colour.s = RandomFloat (0.5, 1);
        colour.l = RandomFloat (std::max (0.0, colour.l - 0.25),
                                std::min (1.0, colour.l + 0.25));

        CARGB newColour = colour.ToARGB ();

        CHud__SetHudColour (index, newColour.r, newColour.g, newColour.b, a);
    }

    /*******************************************************/
    static uint32_t
    RandomizeVehicleColour (CCustomShaderEffectVehicle *shader, CVehicle *veh)
    {
        uint32_t ret = CCustomShaderEffectVehicle_SetForVehicle_134 (shader, veh);
        CARGB *  colours = shader->GetColours ();

        for (int i = 0; i < 4; i++)
            {
                using Rainbomizer::HSL;
                
                colours[i]
                    = HSL (RandomFloat (360), 1.0, RandomFloat (1.0)).ToARGB ();
            }

        return ret;
    }

public:
    /*******************************************************/
    ColoursRandomizer ()
    {
        bool RandomizeHudColours = true;
        bool RandomizeCarColours = true;

        if (!ConfigManager::ReadConfig (
                "ColourRandomizer",
                std::pair ("RandomizeHudColours", &RandomizeHudColours),
                std::pair ("RandomizeCarColours", &RandomizeCarColours)))
            return;

        InitialiseAllComponents ();

        // Hud Colours
        // ----------
        if (RandomizeHudColours)
            RegisterHook ("8b ? ? ? ? ? 8b ? ? ? ? ? 8b cb 89 44 ? ? e8", 18,
                          CHud__SetHudColour, SetNewHudColour);


        // Car Colours
        // ---------
        if (RandomizeCarColours)
            {
                auto addr = GetPatterns (
                    {{"08 55 53 56 57 ? 54 ? 55 ? 56 ? 57 ? 8b ec "
                      "? 83 ec 58 ? 83 c8 ff",
                      -4},
                     {"40 55 53 56 57 ? 54 ? 55 ? 56 ? 57 ? 8b ec "
                      "? 83 ec 58 ? 83 c8 ff",
                      0}});

                RegisterJmpHook<13> (
                    addr, CCustomShaderEffectVehicle_SetForVehicle_134,
                    RandomizeVehicleColour);
            }
    }
} _cols;
