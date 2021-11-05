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
#include <future>

#ifdef ENABLE_DEBUG_MENU
#include <debug/base.hh>
#endif

class WeatherRandomizer
{

    // Config
    /*******************************************************/
    RB_C_CONFIG_START
    {
        bool        RandomizeWeather       = true;
        bool        RandomizeTimecycle     = true;
        bool        CrazyMode              = false;
        bool        RandomizeEveryFade     = true;
        double      RandomizeTimeOdds      = 75.0;
        double      RandomizeTimecycleOdds = 25.0;
        std::string TunableFile            = "Timecyc/Default.txt";
    }
    RB_C_CONFIG_END

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

        static std::future<void> future;

        if (future.valid ())
            future.wait ();

        static std::array TimecyclePresets = {
            "Timecyc/Default.txt",
            "Timecyc/Alternative.txt",
            "Timecyc/SkyAndColours.txt",
            "Timecyc/SkyOnly.txt",
        };

        future = std::async (std::launch::async, [restoreTimecycles] {
            if (restoreTimecycles)
                WeatherRandomizer_TunableManager::RestoreOriginalTimecycles ();

            WeatherRandomizer_TunableManager::Initialise (
                GetRandomElement (TimecyclePresets));

            if (RandomFloat (1000) < Config ().RandomizeTimecycleOdds * 10.0f)
                WeatherRandomizer_TunableManager::Randomize ();
            else if (RandomFloat (1000) < Config ().RandomizeTimeOdds * 10.0f)
                WeatherRandomizer_TunableManager::RandomizeTimesamples ();
        });
    }

public:
    /*******************************************************/
    WeatherRandomizer ()
    {
        if (!ConfigManager ::ReadConfig (
                "TimecycleRandomizer",
                std ::make_pair ("RandomizeWeather",
                                 &Config ().RandomizeWeather),
                std ::make_pair ("RandomizeTimecycle",
                                 &Config ().RandomizeTimecycle),
                std ::make_pair ("TunableFile", &Config ().TunableFile),
                std ::make_pair ("RandomizeTimecycleOdds",
                                 &Config ().RandomizeTimecycleOdds),
                std ::make_pair ("RandomizeTimeOdds",
                                 &Config ().RandomizeTimeOdds),
                std ::make_pair ("RandomizeEveryFade",
                                 &Config ().RandomizeEveryFade)))
            return;
        ;

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

                Rainbomizer::Logger::LogMessage ("Checking Every Fade");
                if (Config ().RandomizeEveryFade)
                    Rainbomizer::Events ().OnFade += RandomizeWeather;
            }

        if (Config ().RandomizeTimecycle)
            {
                Rainbomizer::Events ().OnInit
                    += std::bind (RandomizeTimecycles, false);

                if (Config ().RandomizeEveryFade)
                    Rainbomizer::Events ().OnFade
                        += std::bind (RandomizeTimecycles, true);
            }
    }
} _weather;
