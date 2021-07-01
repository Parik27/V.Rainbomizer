#include "CTimecycle.hh"
#include "Utils.hh"
#include "base.hh"
#include "common/common.hh"
#include "fmt/core.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "rage.hh"
#include <climits>
#include <cstdint>
#include <map>
#include <random>
#include <stdio.h>
#include <string>
#include <vector>
#include <array>
#include <HSL.hh>
#include <fmt/format.h>
#include <Utils.hh>

class SpecialBool
{
    bool check = false;

public:
    operator bool () { return check; }

    bool
    operator= (bool other)
    {
        if (other)
            check = true;

        return other;
    }
};

struct TimecycleRandomizerTunables
{
    inline static std::mt19937                            sm_Random;
    inline static std::vector<std::array<uint8_t, 22464>> sm_Backup;

    static size_t
    RandomSize (size_t size)
    {
        std::uniform_int_distribution<size_t> d{0, size};
        return d (sm_Random);
    }

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

    template <typename T>
    static auto &
    GetRandomElement (const T &container)
    {
        auto it = std::begin (container);
        std::advance (it, RandomSize (std::size (container) - 1));

        return *it;
    }

    template <typename T>
    static auto &
    GetRandomElementMut (T &container)
    {
        auto it = std::begin (container);
        std::advance (it, RandomSize (std::size (container) - 1));

        return *it;
    }

    static float
    RandomFloat (float min, float max)
    {
        if (min > max)
            return max;

        std::uniform_real_distribution<float> d{min, max};
        return d (sm_Random);
    }

    static auto &
    RandomTimeSample ()
    {

        uint32_t totalTimecycles = tcConfig::ms_cycleInfo->Size;

        auto &timecyc = tcManager::g_timeCycle
                            ->pTimecycles[RandomSize (totalTimecycles - 1)];
        auto &region     = GetRandomElementMut (timecyc.Regions);
        auto &timesample = GetRandomElementMut (region.TimeSamples);

        return timesample;
    }

    struct Data
    {
        enum class RandomType : int
        {
            NONE = 0,
            SHUFFLE,
            LIST,
            RANGE
        };

        union
        {
            RandomType Type = RandomType::NONE;
            int        Type_int;
        };

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
            return GetRandomElementMut (ValueList);
        }

        rage::float16
        RandomRange ()
        {
            if (MinValue < MaxValue)
                return RandomFloat (MinValue, MaxValue);
            return RandomFloat (MaxValue, MaxValue);
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
    };

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

        union
        {
            RandomType Type = RandomType::SHUFFLE_COLOR_RANDOM_HSL;
            int        Type_int;
        };

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
            const auto &value = GetRandomElementMut (ValueList);
            out[0]            = value[0];
            out[1]            = value[1];
            out[2]            = value[2];
        }

        void
        RandomTrueColor (rage::float16 *out)
        {
            using Rainbomizer::HSL;

            HSL colour (RandomFloat (MinHue, MaxHue),
                        RandomFloat (MinSat, MaxSat),
                        RandomFloat (MinLue, MaxLue));

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

            colour.h = RandomFloat (0.0f, 360);
            colour.s = RandomFloat (0.5f, 1.0f);
            colour.l = RandomFloat (std::max (0.0f, colour.l - 0.25f),
                                    std::min (1.0f, colour.l + 0.25f));

            CARGB newColour = colour.ToARGB ();
            out[0]          = newColour.r / 255.0f;
            out[1]          = newColour.g / 255.0f;
            out[2]          = newColour.b / 255.0f;
        }

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

        DataColour (uint32_t index) : VariableIndex (index) {}
    };

    inline static std::vector<Data>       sm_Fields;
    inline static std::vector<DataColour> sm_ColourFields;

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

    static void
    Initialise ()
    {
        static bool bInitialised = false;
        if (bInitialised)
            return;

        for (int i = 0; i < tcConfig::GetNumVars (); i++)
            {
                if (IsTimecycVarColourVar (i))
                    {
                        sm_ColourFields.push_back (i);
                        i += 2;
                    }
                else
                    sm_Fields.push_back (i);
            }

        for (int i = 0; i < tcConfig::ms_cycleInfo->Size; i++)
            BackupTimecycle (tcManager::g_timeCycle->pTimecycles[i]);

        bInitialised = true;
    }

    static void
    RandomizeTimecycle (tcCycleInfo &cycle)
    {
        for (auto &region : cycle.Regions)
            {
                for (auto &timeSample : region.TimeSamples)
                    {
                        for (auto i : sm_Fields)
                            i.Randomize (
                                &timeSample.Variables[i.VariableIndex]);
                        for (auto i : sm_ColourFields)
                            i.Randomize (
                                &timeSample.Variables[i.VariableIndex]);
                    }
            }
    }

    static void
    Randomize (uint32_t seed)
    {
        sm_Random.seed (seed);
        for (int i = 0; i < tcConfig::ms_cycleInfo->Size; i++)
            {
                RestoreTimecycle (tcManager::g_timeCycle->pTimecycles[i], i);
            }

        for (int i = 0; i < tcConfig::ms_cycleInfo->Size; i++)
            {
                RandomizeTimecycle (tcManager::g_timeCycle->pTimecycles[i]);
            }
    }

    static void
    Save (const std::string &fileName)
    {
        FILE *file
            = Rainbomizer::Common::GetRainbomizerFile (fileName.data (), "w");

        if (!file)
            return;

        for (auto &i : sm_Fields)
            {
                char *varName
                    = tcConfig::GetVarInfos ()[i.VariableIndex].sAttributeName;
                fmt::print (file, "{} FIELD {} ", varName, i.Type);

                switch (i.Type)
                    {
                    case Data::RandomType::NONE:
                    case Data::RandomType::SHUFFLE:
                    case Data::RandomType::LIST:
                        fmt::print (file, " {}", i.ValueList.size ());

                        for (auto i : i.ValueList)
                            fmt::print (file, " {}", i);

                        break;

                    case Data::RandomType::RANGE:
                        fmt::print (file, " {} {}", i.MinValue, i.MaxValue);
                        break;
                    }

                fputc ('\n', file);
            }

        for (auto &i : sm_ColourFields)
            {
                char *varName
                    = tcConfig::GetVarInfos ()[i.VariableIndex].sAttributeName;
                fmt::print (file, "{} FIELD_COL {}", varName, i.Type);

                switch (i.Type)
                    {
                    case DataColour::RandomType::NONE:
                    case DataColour::RandomType::SHUFFLE_FIELD:
                    case DataColour::RandomType::SHUFFLE_COLOR:
                    case DataColour::RandomType::RANDOM_COLOR_HSL:
                    case DataColour::RandomType::SHUFFLE_COLOR_RANDOM_HSL:
                        break;
                    case DataColour::RandomType::LIST:
                        fmt::print (file, " {}", i.ValueList.size ());

                        for (auto i : i.ValueList)
                            fmt::print (file, " {} {} {}", i[0], i[1], i[2]);

                        break;

                    case DataColour::RandomType::RANDOM_COLOR:
                        fmt::print (file, " {} {} {} {} {} {}", i.MinHue,
                                    i.MaxHue, i.MinSat, i.MaxSat, i.MinLue,
                                    i.MaxLue);
                        break;
                    }

                fputc ('\n', file);
            }

        fclose (file);
    }

    static void
    Load (const std::string &fileName)
    {
        FILE *file
            = Rainbomizer::Common::GetRainbomizerFile (fileName.data (), "r");

        if (!file)
            return;

        std::map<std::string, std::string> lines;

        char line[2048];
        while (fgets (line, 2048, file))
            {
                char name[512] = {0};
                sscanf (line, " %s ", name);

                lines[name] = line;
            }

        for (auto &i : sm_Fields)
            {
                char *varName
                    = tcConfig::GetVarInfos ()[i.VariableIndex].sAttributeName;

                if (auto line = LookupMap (lines, varName))
                    {
                        sscanf (line->c_str (), " %*s %*s %d ", &i.Type_int);
                        switch (i.Type)
                            {
                            case Data::RandomType::NONE:
                            case Data::RandomType::SHUFFLE: break;
                            case Data::RandomType::LIST: break;
                            case Data::RandomType::RANGE:
                                sscanf (line->c_str (), " %*s %*s %*d %f %f",
                                        &i.MinValue, &i.MaxValue);
                            }
                    }
            }

        for (auto &i : sm_ColourFields)
            {
                char *varName
                    = tcConfig::GetVarInfos ()[i.VariableIndex].sAttributeName;

                if (auto line = LookupMap (lines, varName))
                    {
                        sscanf (line->c_str (), " %*s %*s %d ", &i.Type_int);

                        if (i.Type == DataColour::RandomType::RANDOM_COLOR)
                            {
                                sscanf (line->c_str (),
                                        " %*s %*s %*d %f %f %f %f %f %f",
                                        &i.MinHue, &i.MaxHue, &i.MinSat,
                                        &i.MaxSat, &i.MinLue, &i.MaxLue);
                            }
                    }
            }
    }
};

class TimecycleDebugInterface : public DebugInterface
{

    auto &
    GetBaseTimecycle ()
    {
        return tcManager::g_timeCycle->pTimecycles->Regions[0].TimeSamples[0];
    }

    void
    OverrideAllTimecycles ()
    {
        uint32_t totalTimecycles = tcConfig::ms_cycleInfo->Size;

        auto &base = GetBaseTimecycle ();

        for (int i = 0; i < totalTimecycles; i++)
            {
                for (auto &region :
                     tcManager::g_timeCycle->pTimecycles[i].Regions)
                    {
                        for (auto &sample : region.TimeSamples)
                            {
                                memcpy (sample.Variables, base.Variables,
                                        sizeof (base.Variables));
                            }
                    }
            }
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

    void
    DrawTesting ()
    {
        static float aTestingVars[432];

        if (!tcManager::g_timeCycle || !tcManager::g_timeCycle->pTimecycles)
            return;

        static ImGuiTextFilter filter;

        auto &base = GetBaseTimecycle ();
        for (int i = 0; i < tcConfig::GetNumVars (); i++)
            aTestingVars[i] = base.Variables[i].to_float ();

        filter.Draw ();

        ImGui::Columns (2);
        for (int i = 0; i < tcConfig::GetNumVars (); i++)
            {
                char *name = tcConfig::GetVarInfos ()[i].sAttributeName;
                if (!filter.PassFilter (name))
                    continue;

                ImGui::Text ("%s", name);
                ImGui::NextColumn ();

                ImGui::PushID (i);
                if (IsTimecycVarColourVar (i))
                    {
                        if (ImGui::ColorEdit3 ("", &aTestingVars[i]))
                            {
                                base.Variables[i]     = aTestingVars[i];
                                base.Variables[i + 1] = aTestingVars[i + 1];
                                base.Variables[i + 2] = aTestingVars[i + 2];
                            }

                        i += 2;
                    }
                else
                    {
                        if (ImGui::InputFloat ("", &aTestingVars[i]))
                            {
                                base.Variables[i] = aTestingVars[i];
                            }
                    }

                ImGui::PopID ();
                ImGui::NextColumn ();
            }

        OverrideAllTimecycles ();
    }

    void
    DrawNewTesting ()
    {
        SpecialBool            changed;
        static uint32_t        seed     = 0;
        static std::string     fileName = "";
        static ImGuiTextFilter filter;

        TimecycleRandomizerTunables::Initialise ();

        changed = ImGui::SliderInt ("Seed", (int *) &seed, 0, 10000);

        filter.Draw ();

        ImGui::InputText ("##filename", &fileName);
        ImGui::SameLine ();
        if (ImGui::Button ("Save"))
            TimecycleRandomizerTunables::Save (fileName);
        ImGui::SameLine ();
        if ((changed = ImGui::Button ("Load")))
            TimecycleRandomizerTunables::Load (fileName);

        ImGui::BeginChild ("Tunables");
        ImGui::Columns (2);
        for (auto &i : TimecycleRandomizerTunables::sm_Fields)
            {
                char *varName
                    = tcConfig::GetVarInfos ()[i.VariableIndex].sAttributeName;

                if (!filter.PassFilter (varName))
                    continue;

                ImGui::Text ("%s", varName);
                ImGui::NextColumn ();

                ImGui::PushID (i.VariableIndex);
                changed = ImGui::Combo ("##Type", &i.Type_int,
                                        "NONE\0SHUFFLE\0LIST\0RANGE\0");
                switch (i.Type)
                    {

                    case TimecycleRandomizerTunables::Data::RandomType::NONE:
                    case TimecycleRandomizerTunables::Data::RandomType::SHUFFLE:
                        break;

                    case TimecycleRandomizerTunables::Data::RandomType::LIST:
                        if ((changed = ImGui::Button ("+")))
                            i.ValueList.push_back (0.0f);
                        ImGui::SameLine ();

                        if ((changed = ImGui::Button ("-"))
                            && i.ValueList.size () > 1)
                            i.ValueList.pop_back ();

                        for (int j = 0; j < i.ValueList.size (); j++)
                            {
                                ImGui::PushID (j);
                                changed
                                    = ImGui::InputFloat ("", &i.ValueList[j]);
                                ImGui::PopID ();
                            }
                        break;

                    case TimecycleRandomizerTunables::Data::RandomType::RANGE:

                        changed = ImGui::InputFloat ("##Min", &i.MinValue);
                        changed = ImGui::InputFloat ("##Max", &i.MaxValue);
                        break;
                    }

                ImGui::PopID ();
                ImGui::NextColumn ();
            }

        for (auto &i : TimecycleRandomizerTunables::sm_ColourFields)
            {
                char *varName
                    = tcConfig::GetVarInfos ()[i.VariableIndex].sAttributeName;

                if (!filter.PassFilter (varName))
                    continue;

                ImGui::Text ("%s", varName);
                ImGui::NextColumn ();

                ImGui::PushID (i.VariableIndex);
                changed = ImGui::Combo ("##Type", &i.Type_int,
                                        "NONE\0"
                                        "SHUFFLE_FIELD\0"
                                        "SHUFFLE_COLOR\0"
                                        "LIST\0"
                                        "RANDOM_COLOR\0"
                                        "RANDOM_COLOR_HSL\0"
                                        "SHUFFLE_COLOR_RANDOM_HSL\0");

                switch (i.Type)
                    {

                    case TimecycleRandomizerTunables::DataColour::RandomType::
                        NONE:
                    case TimecycleRandomizerTunables::DataColour::RandomType::
                        SHUFFLE_FIELD:
                    case TimecycleRandomizerTunables::DataColour::RandomType::
                        SHUFFLE_COLOR:

                    case TimecycleRandomizerTunables::DataColour::RandomType::
                        RANDOM_COLOR_HSL:
                    case TimecycleRandomizerTunables::DataColour::RandomType::
                        SHUFFLE_COLOR_RANDOM_HSL:
                        break;

                    case TimecycleRandomizerTunables::DataColour::RandomType::
                        LIST:
                        if ((changed = ImGui::Button ("+")))
                            i.ValueList.push_back ({0.5f, 0.5f, 0.5f});
                        if ((changed = ImGui::Button ("-"))
                            && i.ValueList.size () > 1)
                            i.ValueList.pop_back ();

                        for (int j = 0; j < i.ValueList.size (); j++)
                            {
                                ImGui::PushID (j);
                                ImGui::SameLine ();
                                changed = ImGui::ColorEdit3 (
                                    "", i.ValueList[j].data (),
                                    ImGuiColorEditFlags_NoInputs);
                                ImGui::PopID ();
                            }
                        break;

                    case TimecycleRandomizerTunables::DataColour::RandomType::
                        RANDOM_COLOR:

                        changed = ImGui::SliderFloat ("Min Hue", &i.MinHue,
                                                      0.0f, 360.0f);
                        changed = ImGui::SliderFloat ("Max Hue", &i.MaxHue,
                                                      0.0f, 360.0f);
                        changed = ImGui::SliderFloat ("Min Sat", &i.MinSat,
                                                      0.0f, 1.0f);
                        changed = ImGui::SliderFloat ("Max Sat", &i.MaxSat,
                                                      0.0f, 1.0f);
                        changed = ImGui::SliderFloat ("Min Lue", &i.MinLue,
                                                      0.0f, 1.0f);
                        changed = ImGui::SliderFloat ("Max Lue", &i.MaxLue,
                                                      0.0f, 1.0f);

                        break;
                    }

                ImGui::PopID ();
                ImGui::NextColumn ();
            }

        ImGui::EndChild ();

        if (changed)
            TimecycleRandomizerTunables::Randomize (seed);
    }

    void
    Draw () override
    {
        static bool bEnableTesting;
        static bool bEnableNewTesting;

        ImGui::Checkbox ("Enable Testing", &bEnableTesting);
        ImGui::Checkbox ("Enable New Testing", &bEnableNewTesting);

        if (bEnableTesting)
            DrawTesting ();

        else if (bEnableNewTesting)
            DrawNewTesting ();
    }

public:
    const char *
    GetName () override
    {
        return "Timecycle";
    }

} g_TimecycleDebugInterface;
