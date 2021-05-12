#include <Utils.hh>
#include <common/common.hh>
#include <CWeather.hh>
#include <cstring>
#include <array>
#include <common/logger.hh>
#include "CTimecycle.hh"
#include <common/config.hh>

class WeatherRandomizer
{

    // Config
    static inline struct Config
    {
        bool RandomizeWeather   = false;
        bool RandomizeTimecycle = true;
        bool CrazyMode          = false;

        Config (){};

    } m_Config;

    /*******************************************************/
    static void
    RandomizeWeather (bool session)
    {
        // This is not really necessary, just means that there'll be an
        // additional unnessary randomization when the game is loaded.
        if (!session)
            return;

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
    static double
    RandomizeTimecycleComponent (const char *value)
    {
        float flVal = atof (value);

        auto RandomRanged = [&flVal] (float min, float max) {
            if (flVal >= min && flVal <= max)
                {
                    flVal = RandomFloat (min, max);
                    return true;
                }

            return false;
        };

        // This code is like -
        // If original is in range 0 - 1, return random between that range,
        // repeat for 0 - 5 and so on.
        std::array Ranges = {1.0f, 5.0f, 10.0f, 50.0f};

        for (auto i : Ranges)
            if (RandomRanged (0.0f, i) || RandomRanged (-i, 0.0f))
                break;

        return flVal;
    }

    /*******************************************************/
    static bool
    IsTimecycVarColourVar (uint32_t index)
    {
        char *varName = tcConfig::GetVarInfos ()[index].sAttributeName;
        // all colour variables end with _r, _g, or _b - so
        // comparing with last 2 characters.
        varName += strlen (varName) - 2;

        return strcmp (varName, "_r") == 0 || strcmp (varName, "_g") == 0
               || strcmp (varName, "_b") == 0;
    }

    /*******************************************************/
    static rage::float16 &
    GetRandomTimecycleVariable (uint32_t variableIndex)
    {
        uint32_t totalTimecycles = tcConfig::ms_cycleInfo->Size;

        auto &timecyc = tcManager::g_timeCycle
                            ->pTimecycles[RandomInt (totalTimecycles - 1)];
        auto &region     = GetRandomElementMut (timecyc.Regions);
        auto &timesample = GetRandomElementMut (region.TimeSamples);

        return timesample.Variables[variableIndex];
    }

    /*******************************************************/
    static void
    RandomizeTimecycle (tcCycleInfo &cycle)
    {
        for (auto &region : cycle.Regions)
            {
                for (auto &timeSample : region.TimeSamples)
                    {
                        uint32_t variableIndex = 0;
                        for (auto &variable : timeSample.Variables)
                            {
                                if (variableIndex >= tcConfig::GetNumVars ())
                                    break;

                                std::swap (variable,
                                           GetRandomTimecycleVariable (
                                               variableIndex));

                                variableIndex++;
                            }
                    }
            }
    }

    /*******************************************************/
    static void
    RandomizeTimecycles (bool session)
    {
        if (!session)
            return;

        // Shouldn't happen? But maybe for like future updates if (unlikely) the
        // offset for pTimecycles changes.
        if (!tcManager::g_timeCycle->pTimecycles)
            return;

        for (int i = 0; i < tcConfig::ms_cycleInfo->Size; i++)
            RandomizeTimecycle (tcManager::g_timeCycle->pTimecycles[i]);
    }

public:
    /*******************************************************/
    WeatherRandomizer ()
    {
        if (!ConfigManager::ReadConfig (
                "WeatherRandomizer",
                std::pair ("RandomizeWeather", &m_Config.RandomizeWeather),
                std::pair ("RandomizeTimecycle", &m_Config.RandomizeTimecycle),
                std::pair ("CrazyMode", &m_Config.CrazyMode)))
            return;

        InitialiseAllComponents ();

        if (m_Config.RandomizeWeather)
            Rainbomizer::Common::AddInitCallback (RandomizeWeather);

        if (!m_Config.CrazyMode)
            Rainbomizer::Common::AddInitCallback (RandomizeTimecycles);
        else
            RegisterHook (
                "? 03 de e8 ? ? ? ? b9 00 00 00 38 0f 57 c9 f2 0f 5a c8", 3,
                RandomizeTimecycleComponent);
    }
} _weather;
