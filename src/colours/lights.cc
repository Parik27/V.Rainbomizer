#include <common/config.hh>
#include <common/events.hh>
#include <common/minhook.hh>

#ifdef ENABLE_DEBUG_MENU
#include <debug/base.hh>
#endif

#include "HSL.hh"
#include "CARGB.hh"
#include "rage.hh"

#include <Utils.hh>

class CCoronas;

struct LightConeInfo
{
    float      PosX;
    float      PosY;
    float      PosZ;
    float      PosW;
    ColorFloat Color;
    float      Alpha;
    uint64_t   field_0x20;
    uint32_t   field_0x28;
    uint32_t   field_0x2c;
    uint32_t   field_0x30;
    uint64_t   field_0x34;
    uint32_t   field_0x3c;
    uint32_t   field_0x40;
    uint32_t   field_0x44;
    uint32_t   field_0x48;
    uint32_t   field_0x4c;
    uint32_t   field_0x50;
    uint32_t   field_0x54;
    uint32_t   field_0x58;
    uint32_t   field_0x5c;
    uint32_t   nFlags;
    uint32_t   field_0x64;
    float      fIntensity;
};

class LightsRandomizer
{
    RB_C_CONFIG_START
    {
        double LightShiftFrequency = 1.0f;
        double RandomizeOdds       = 85.0f;
    }
    RB_C_CONFIG_END

    inline static bool  bRandomize       = true;
    inline static float LightsHueShift   = RandomFloat (360);
    inline static float LightsSaturation = 1.0;
    inline static float LightsLumi       = 0.5;

public:
    /*******************************************************/
    template <auto &ReadColourRGBA>
    static float *
    RandomizeLight (void *file, float *output, char *name)
    {
        float *out = ReadColourRGBA (file, output, name);
        out[0]     = 1.0f;
        out[1]     = 0.0f;
        out[2]     = 0.0f;
        out[3]     = 1.0f;

        return out;
    }

    /*******************************************************/
    static void
    AdjustLightColour (Rainbomizer::HSL &colour, float PosX, float PosY,
                       bool Rainbow = true)
    {
        if (!bRandomize)
            return;

        colour.h += LightsHueShift;
        if (Config ().LightShiftFrequency > 0.01)
            colour.h += fabs ((PosX + PosY) / Config ().LightShiftFrequency);
        colour.h = fmod (colour.h, 360.0f);

        if (Rainbow)
            {
                colour.s = LightsSaturation;
                colour.l = LightsLumi;
            }
    }

    /*******************************************************/
    template <auto &_AddLightCone>
    static bool
    RandomizeLightCone (LightConeInfo *info, void *p2, bool p3)
    {
        using Rainbomizer::HSL;

        HSL colour = info->Color;

        AdjustLightColour (colour, info->PosX, info->PosY);

        info->Color = colour;
        return _AddLightCone (info, p2, p3);
    }

    /*******************************************************/
    template <auto &CCoronas__Add>
    static void
    RandomizeCorona (CCoronas *p1, rage::Vec3V *pos, float size, CARGB colour,
                     float intensity, float zBias, float *p7, uint32_t p8,
                     float p9, float p10, uint16_t p11)
    {
        using Rainbomizer::HSL;

        HSL hsl{colour};
        AdjustLightColour (hsl, pos->x, pos->y);

        colour = hsl;
        return CCoronas__Add (p1, pos, size, colour, intensity, zBias, p7, p8,
                              p9, p10, p11);
    }

    /*******************************************************/
    static void
    RandomizeOnFade ()
    {
        LightsHueShift   = RandomFloat (360);
        LightsLumi       = RandomFloat (0.3, 0.7);
        LightsSaturation = RandomFloat (0.7, 1.0);

        bRandomize = RandomFloat (100) < Config ().RandomizeOdds;
    }

    /*******************************************************/
    LightsRandomizer ()
    {
        if (!ConfigManager ::ReadConfig (
                "LightRandomizer",
                std::make_pair ("LightShiftFrequency",
                                &Config ().LightShiftFrequency),
                std::make_pair ("RandomizeOdds", &Config ().RandomizeOdds)))
            return;
        ;

        InitialiseAllComponents ();

#ifdef RANDOMIZE_VISUAL_SETTINGS_COLOURS
        REGISTER_MH_HOOK_BRANCH ("0f 45 44 ? ? ? 8d ? ? 40 ? 8b cd e8 ", 13,
                                 RandomizeLight, float *, void *, float *,
                                 char *);
#endif

        REGISTER_MH_HOOK ("? 8d ? a1 ? 81 ec a0 00 00 00 0f 29 ? ? 45 33 e4 0f "
                          "29 ? ? 44 38 25",
                          -24, RandomizeLightCone, bool, LightConeInfo *,
                          void *, bool);

        REGISTER_MH_HOOK ("44 89 4c ? ? ? 83 ec ? 0f 29 74 ? ? 0f 57 c0 "
                          "0f 29 3c ? ? 8b d2 ? 8b c9 0f 28 fa 0f 2e f8 ",
                          0, RandomizeCorona, void, CCoronas *, rage::Vec3V *,
                          float, CARGB, float, float, float *, uint32_t, float,
                          float, uint16_t);

        Rainbomizer::Events ().OnFade += RandomizeOnFade;

#ifdef ENABLE_DEBUG_MENU
        DebugInterfaceManager::AddAction ("Randomize Lights",
                                          std::bind (RandomizeOnFade));
#endif
    }
} _lights;
