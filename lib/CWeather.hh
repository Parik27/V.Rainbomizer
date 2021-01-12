#pragma once

#include <cstdint>

//
struct CWeatherType
{
    uint32_t nHash;
    float    Sun;
    float    Cloud;
    float    WindMin;
    float    WindMax;
    float    Rain;
    float    Snow;
    float    SnowMist;
    float    Fog;
    bool     Lightning;
    bool     Sandstorm;
    uint32_t DropSettingIndex;
    uint32_t MistSettingIndex;
    uint32_t GroundSettingsIndex;
    float    RippleBumpiness;
    float    RippleMinBumpiness;
    float    RippleMaxBumpiness;
    float    RippleBumpinessWindScale;
    float    RippleSpeed;
    float    RippleDisturb;
    float    RippleVelocityTransfer;
    float    OceanBumpiness;
    float    DeepOceanScale;
    float    OceanNoiseMinAmplitude;
    float    OceanWaveAmplitude;
    float    ShoreWaveAmplitude;
    float    OceanWaveWindScale;
    float    ShoreWaveWindScale;
    float    OceanWaveMinAmplitude;
    float    ShoreWaveMinAmplitude;
    float    OceanWaveMaxAmplitude;
    float    ShoreWaveMaxAmplitude;
    float    OceanFoamIntensity;
    char     TimecycleFileName[64];
    uint32_t field_0xc0;
};

static_assert (sizeof (CWeatherType) == 196, "Check align for CWeatherType");

class CWeather
{
public:
    int                 nTotalTypes;
    struct CWeatherType aWeatherTypes[14];
    // Has a lot more elements, but that's all I need for now.

    inline static CWeather *g_Weather;

    static void InitialisePatterns ();
};
