#include <Utils.hh>
#include <common/events.hh>
#include <CWeather.hh>
#include <cstring>
#include <array>
#include <common/logger.hh>
#include "CTimecycle.hh"
#include <common/config.hh>
#include <mutex>
#include "weather_Timecycle.hh"

#ifdef ENABLE_DEBUG_MENU
#include <debug/base.hh>
#endif

class WeatherRandomizer
{

    // Config
    static auto &
    Config ()
    {
        static struct Config
        {
            bool        RandomizeWeather   = true;
            bool        RandomizeTimecycle = true;
            bool        CrazyMode          = false;
            std::string TunableFile        = "Timecyc/Default.txt";
        } sm_Config;

        return sm_Config;
    }

    /*******************************************************/
    static void
    RandomizeWeather ()
    {
        auto GetRandomWeather = []() -> auto &
        {
            return CWeather::g_Weather->aWeatherTypes[RandomInt (
                CWeather::g_Weather->nTotalTypes - 1)];
        };

        for (int i = 0; i < CWeather::g_Weather->nTotalTypes; i++)
            {
                CWeatherType &type = CWeather::g_Weather->aWeatherTypes[i];

                // Frankenstein randomization, basically. This will swap each
                // field for a certain element with the field of a random
                // element. So you could have "Sun" field from cloudy and "Rain"
                // field from sunny for example.

#define RANDOMIZE_FIELD(field)                                                 \
    std::swap (type.field, GetRandomWeather ().field);

                RANDOMIZE_FIELD (Sun)
                RANDOMIZE_FIELD (Cloud)
                RANDOMIZE_FIELD (WindMin)
                RANDOMIZE_FIELD (WindMax)
                RANDOMIZE_FIELD (Rain)
                RANDOMIZE_FIELD (Snow)
                RANDOMIZE_FIELD (SnowMist)
                RANDOMIZE_FIELD (Fog)
                RANDOMIZE_FIELD (Lightning)
                RANDOMIZE_FIELD (Sandstorm)
                RANDOMIZE_FIELD (DropSettingIndex)
                RANDOMIZE_FIELD (MistSettingIndex)
                RANDOMIZE_FIELD (GroundSettingsIndex)
                RANDOMIZE_FIELD (RippleBumpiness)
                RANDOMIZE_FIELD (RippleMinBumpiness)
                RANDOMIZE_FIELD (RippleMaxBumpiness)
                RANDOMIZE_FIELD (RippleBumpinessWindScale)
                RANDOMIZE_FIELD (RippleSpeed)
                RANDOMIZE_FIELD (RippleDisturb)
                RANDOMIZE_FIELD (RippleVelocityTransfer)
                RANDOMIZE_FIELD (OceanBumpiness)
                RANDOMIZE_FIELD (DeepOceanScale)
                RANDOMIZE_FIELD (OceanNoiseMinAmplitude)
                RANDOMIZE_FIELD (OceanWaveAmplitude)
                RANDOMIZE_FIELD (ShoreWaveAmplitude)
                RANDOMIZE_FIELD (OceanWaveWindScale)
                RANDOMIZE_FIELD (ShoreWaveWindScale)
                RANDOMIZE_FIELD (OceanWaveMinAmplitude)
                RANDOMIZE_FIELD (ShoreWaveMinAmplitude)
                RANDOMIZE_FIELD (OceanWaveMaxAmplitude)
                RANDOMIZE_FIELD (ShoreWaveMaxAmplitude)
                RANDOMIZE_FIELD (OceanFoamIntensity)
                RANDOMIZE_FIELD (TimecycleFileName)

#undef RANDOMIZE_FIELD
            }
    }

    /*******************************************************/
    static void
    RandomizeTimecycles (bool restoreTimecycles)
    {
        // Shouldn't happen? But maybe for like future updates if (unlikely) the
        // offset for pTimecycles changes.
        if (!tcManager::g_timeCycle->pTimecycles)
            return;

        if (restoreTimecycles)
            WeatherRandomizer_TunableManager::RestoreOriginalTimecycles ();

        WeatherRandomizer_TunableManager::Initialise (Config ().TunableFile);
        WeatherRandomizer_TunableManager::Randomize ();
    }

public:
    /*******************************************************/
    WeatherRandomizer ()
    {
        if (!ConfigManager::ReadConfig (
                "WeatherRandomizer",
                std::pair ("RandomizeWeather", &Config ().RandomizeWeather),
                std::pair ("RandomizeTimecycle", &Config ().RandomizeTimecycle),
                std::pair ("TunableFile", &Config ().TunableFile)))
            return;

        InitialiseAllComponents ();

#ifdef ENABLE_DEBUG_MENU
        DebugInterfaceManager::AddAction ("Randomize Weather",
                                          std::bind (RandomizeWeather));
        DebugInterfaceManager::AddAction ("Randomize Timecycle",
                                          RandomizeTimecycles);
#endif

        if (Config ().RandomizeWeather)
            {
                Rainbomizer::Events ().OnInit += std::bind (RandomizeWeather);
                Rainbomizer::Events ().OnFade += RandomizeWeather;
            }

        if (Config ().RandomizeTimecycle)
            {
                Rainbomizer::Events ().OnInit
                    += std::bind (RandomizeTimecycles, false);
                Rainbomizer::Events ().OnFade
                    += std::bind (RandomizeTimecycles, true);
            }
    }
} _weather;
