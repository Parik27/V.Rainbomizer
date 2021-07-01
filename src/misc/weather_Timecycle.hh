#include "common/common.hh"
#include <Random.hh>
#include <CTimecycle.hh>
#include <HSL.hh>

#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>
#include <array>

struct WeatherRandomizer_Tunables
{
    inline static RandomUtils<> sm_Random;

    /*******************************************************/
    static auto &
    RandomTimeSample ()
    {

        uint32_t totalTimecycles = tcConfig::ms_cycleInfo->Size;

        auto &timecyc = tcManager::g_timeCycle
                            ->pTimecycles[sm_Random.Get (totalTimecycles - 1)];
        auto &region     = sm_Random.ElementMut (timecyc.Regions);
        auto &timesample = sm_Random.ElementMut (region.TimeSamples);

        return timesample;
    }

    /*******************************************************/
    static uint32_t
    GetIndexFromFieldName (std::string_view s)
    {
        for (uint32_t i = 0; i < tcConfig::GetNumVars (); i++)
            if (tcConfig::GetVarInfos ()[i].sAttributeName == s)
                return i;

        return -1;
    }

    /*******************************************************/
    struct Data
    {
        enum class RandomType : int
        {
            NONE = 0,
            SHUFFLE,
            LIST,
            RANGE
        };

        RandomType Type = RandomType::NONE;

        uint32_t VariableIndex;

        float MinValue = 0.0f;
        float MaxValue = 1.0f;

        std::vector<float> ValueList{0.5f};

        rage::float16
        RandomShuffle ()
        {
            return RandomTimeSample ().Variables[VariableIndex];
        }

        rage::float16
        RandomList ()
        {
            return sm_Random.Element (ValueList);
        }

        rage::float16
        RandomRange ()
        {
            if (MinValue < MaxValue)
                return sm_Random.Get (MinValue, MaxValue);
            return sm_Random.Get (MaxValue, MaxValue);
        }

        void
        Randomize (rage::float16 *out)
        {
            auto shuffle = RandomShuffle ();
            auto list    = RandomList ();
            auto range   = RandomRange ();

            switch (Type)
                {
                case RandomType::NONE: break;
                case RandomType::SHUFFLE: *out = shuffle; break;
                case RandomType::LIST: *out = list; break;
                case RandomType::RANGE: *out = range; break;
                }
        }

        Data (uint32_t index) : VariableIndex (index) {}

        Data (char *fieldName, char *data)
        {
            uint32_t bytesRead  = 0;
            this->VariableIndex = GetIndexFromFieldName (fieldName);

            sscanf (&data[bytesRead], " %d %n", &Type, &bytesRead);

            switch (Type)
                {
                case RandomType::RANGE:
                    sscanf (&data[bytesRead], " %f %f %n", &MinValue, &MaxValue,
                            &bytesRead);
                default: break;
                }
        }
    };

    /*******************************************************/
    struct DataColour
    {
        enum class RandomType
        {
            NONE = 0,
            SHUFFLE_FIELD,
            SHUFFLE_COLOR,
            LIST,
            RANDOM_COLOR,
            RANDOM_COLOR_HSL,
            SHUFFLE_COLOR_RANDOM_HSL
        };

        RandomType Type = RandomType::SHUFFLE_COLOR_RANDOM_HSL;

        uint32_t VariableIndex;

        float MinHue = 0.0f;
        float MaxHue = 360.0f;
        float MinSat = 0.0f;
        float MaxSat = 1.0f;
        float MinLue = 0.0f;
        float MaxLue = 1.0f;

        std::vector<std::array<float, 3>> ValueList = {{{0.5f, 0.5f, 0.5f}}};

        void
        RandomShuffle (rage::float16 *out)
        {
            out[0] = RandomTimeSample ().Variables[VariableIndex];
            out[1] = RandomTimeSample ().Variables[VariableIndex + 1];
            out[2] = RandomTimeSample ().Variables[VariableIndex + 2];
        }

        void
        RandomShuffleOrdered (rage::float16 *out)
        {
            auto &timeSample = RandomTimeSample ();
            out[0]           = timeSample.Variables[VariableIndex];
            out[1]           = timeSample.Variables[VariableIndex + 1];
            out[2]           = timeSample.Variables[VariableIndex + 2];
        }

        void
        RandomList (rage::float16 *out)
        {
            const auto &value = sm_Random.Element (ValueList);
            out[0]            = value[0];
            out[1]            = value[1];
            out[2]            = value[2];
        }

        void
        RandomTrueColor (rage::float16 *out)
        {
            using Rainbomizer::HSL;

            HSL colour (sm_Random.Get (MinHue, MaxHue),
                        sm_Random.Get (MinSat, MaxSat),
                        sm_Random.Get (MinLue, MaxLue));

            auto col = colour.ToARGB ();

            out[0] = col.r / 255.0f;
            out[1] = col.g / 255.0f;
            out[2] = col.b / 255.0f;
        }

        void
        RandomColor (rage::float16 *out, bool shuffle = false)
        {
            using Rainbomizer::HSL;

            if (shuffle)
                RandomShuffleOrdered (out);

            HSL colour (CARGB (255, out[0].to_float () * 255,
                               out[1].to_float () * 255,
                               out[2].to_float () * 255));

            colour.h = sm_Random.Get (0.0f, 360.0f);
            colour.s = sm_Random.Get (0.5f, 1.0f);
            colour.l = sm_Random.Get (std::max (0.0f, colour.l - 0.25f),
                                      std::min (1.0f, colour.l + 0.25f));

            CARGB newColour = colour.ToARGB ();
            out[0]          = newColour.r / 255.0f;
            out[1]          = newColour.g / 255.0f;
            out[2]          = newColour.b / 255.0f;
        }

#ifdef ENABLE_DEBUG_MENU
        void
        Randomize (rage::float16 *out)
        {
            rage::float16 tmp[3];
            RandomShuffle (Type == RandomType::SHUFFLE_FIELD ? out : tmp);
            RandomShuffleOrdered (Type == RandomType::SHUFFLE_COLOR ? out
                                                                    : tmp);
            RandomList (Type == RandomType::LIST ? out : tmp);
            RandomTrueColor (Type == RandomType::RANDOM_COLOR ? out : tmp);
            RandomColor (Type == RandomType::RANDOM_COLOR_HSL ? out : tmp);
            RandomColor (Type == RandomType::SHUFFLE_COLOR_RANDOM_HSL ? out
                                                                      : tmp,
                         true);
        }
#else
        void
        Randomize (rage::float16 *out)
        {
            switch (Type)
                {
                case RandomType::NONE: break;
                case RandomType::SHUFFLE_FIELD: RandomShuffle (out); break;
                case RandomType::LIST: RandomList (out); break;
                case RandomType::RANDOM_COLOR: RandomTrueColor (out); break;

                case RandomType::SHUFFLE_COLOR:
                    RandomShuffleOrdered (out);
                    break;

                case RandomType::RANDOM_COLOR_HSL:
                case RandomType::SHUFFLE_COLOR_RANDOM_HSL:
                    RandomColor (out,
                                 Type == RandomType::SHUFFLE_COLOR_RANDOM_HSL);
                    break;
                }
        }
#endif

        DataColour (uint32_t index) : VariableIndex (index) {}

        DataColour (char *fieldName, char *data)
        {
            uint32_t bytesRead  = 0;
            this->VariableIndex = GetIndexFromFieldName (fieldName);

            sscanf (&data[bytesRead], " %d %n", &Type, &bytesRead);

            switch (Type)
                {
                case RandomType::RANDOM_COLOR:
                    sscanf (&data[bytesRead], " %f %f %f %f %f %f %n", &MinHue,
                            &MaxHue, &MinSat, &MaxSat, &MinLue, &MaxLue,
                            &bytesRead);
                default: break;
                }
        }
    };

    std::vector<Data>       m_Fields;
    std::vector<DataColour> m_ColourFields;

    void
    Load (const std::string &file)
    {
        FILE *f = Rainbomizer::Common::GetRainbomizerDataFile (file, "r");

        if (!f)
            return;

        char     line[2048] = {0};
        uint32_t bytesRead  = 0;
        while (fgets (line, 2048, f))
            {
                line[strcspn (line, "\n")] = 0;

                char fieldName[256] = {0};
                char fieldType[32]  = {0};

                sscanf (line, " %s %s %n", fieldName, fieldType, &bytesRead);

                if (fieldType == std::string ("FIELD_COL"))
                    m_ColourFields.emplace_back (fieldName, &line[bytesRead]);
                else if (fieldType == std::string ("FIELD"))
                    m_Fields.emplace_back (fieldName, &line[bytesRead]);
            }
    }

    void
    RandomizeTimecycle (tcCycleInfo &cycle)
    {
        for (auto &region : cycle.Regions)
            {
                for (auto &timeSample : region.TimeSamples)
                    {
                        for (auto i : m_Fields)
                            i.Randomize (
                                &timeSample.Variables[i.VariableIndex]);
                        for (auto i : m_ColourFields)
                            i.Randomize (
                                &timeSample.Variables[i.VariableIndex]);
                    }
            }
    }

    void
    Randomize ()
    {
        for (int i = 0; i < tcConfig::ms_cycleInfo->Size; i++)
            RandomizeTimecycle (tcManager::g_timeCycle->pTimecycles[i]);
    }
};

class WeatherRandomizer_TunableManager
{
    inline static WeatherRandomizer_Tunables              sm_Tunable;
    inline static std::vector<std::array<uint8_t, 22464>> sm_Backup;

    static void
    BackupTimecycle (tcCycleInfo &cycle)
    {
        sm_Backup.push_back ({});
        memcpy (sm_Backup.back ().data (), cycle.Regions,
                sizeof (cycle.Regions));
    }

    static void
    RestoreTimecycle (tcCycleInfo &cycle, int index)
    {
        memcpy (cycle.Regions, sm_Backup[index].data (),
                sizeof (cycle.Regions));
    }

public:
    static void
    RestoreOriginalTimecycles ()
    {
        for (int i = 0; i < tcConfig::ms_cycleInfo->Size; i++)
            RestoreTimecycle (tcManager::g_timeCycle->pTimecycles[i], i);
    }

    static WeatherRandomizer_Tunables &
    GetTunable ()
    {
        return sm_Tunable;
    }

    static void
    Initialise (const std::string &file)
    {
        sm_Tunable.Load (file);

        // Already backed up
        if (sm_Backup.size ())
            return;

        for (int i = 0; i < tcConfig::ms_cycleInfo->Size; i++)
            BackupTimecycle (tcManager::g_timeCycle->pTimecycles[i]);
    }

    static void
    Randomize ()
    {
        WeatherRandomizer_Tunables::sm_Random.GetEngine ().seed (
            static_cast<unsigned int> (time (NULL)));

        GetTunable ().Randomize ();
    }
};
