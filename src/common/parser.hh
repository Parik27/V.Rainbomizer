#pragma once

#include "ParserUtils.hh"
#include "common/common.hh"
#include "common/logger.hh"
#include <cstdint>
#include <map>
#include <stdio.h>
#include <string.h>
#include <type_traits>
#include <tuple>
#include <array>
#include <unordered_map>
#include <utility>

/* A randomizer to randomize a field between its min/max value */
template <typename T> class RangedRandomizer
{
    bool bInitialised = false;

    T Min;
    T Max;

public:
    using Type = T;

    void
    RandomizeObject (T &out) const
    {
        out = RandomFloat (Min, Max);
    }

    void
    AddSample (const T value)
    {
        if (!std::exchange (bInitialised, true))
            Min = Max = value;

        Min = std::min (value, Min);
        Max = std::max (value, Max);
    }
};

template <const char *FileName, auto ValidateFunction = nullptr>
class DataFileBasedModelRandomizer
{
    using Type = uint32_t;

private:
    struct ValueGroup
    {
        std::vector<uint32_t> Values;
        std::vector<double>   Weights;
    };

    bool                    m_Initialised = false;
    std::vector<ValueGroup> m_Groups;

    /*******************************************************/
    bool
    IsModelValid (Type hash)
    {
        if (ValidateFunction == nullptr)
            return true;

        return ValidateFunction (hash);
    }

    /*******************************************************/
    void
    Initialise ()
    {
        if (std::exchange (m_Initialised, true))
            return;

        FILE *file = Rainbomizer::Common::GetRainbomizerDataFile (FileName);
        if (!file)
            return;

        m_Groups.push_back ({});

        char line[512] = {0};
        while (fgets (line, 512, file))
            {
                if (strlen (line) < 3)
                    {
                        m_Groups.push_back ({});
                        continue;
                    }

                double weight     = 1.0;
                char   model[256] = {0};

                sscanf (line, "%s %lf ", model, &weight);

                Type value = rage::atStringHash (model);

                if (IsModelValid (value))
                    {
                        m_Groups.back ().Values.push_back (value);
                        m_Groups.back ().Weights.push_back (weight);
                    }
                else
                    Rainbomizer::Logger::LogMessage ("Invalid model: %s",
                                                     model);
            }
    }

public:
    /*******************************************************/
    bool
    RandomizeObject (Type &out)
    {
        Initialise ();
        for (const auto &i : m_Groups)
            {
                if (DoesElementExist (i.Values, out))
                    {
                        out = i.Values[RandomWeighed (i.Weights)];
                        return true;
                    }
            }

        return false;
    }

    /*******************************************************/
    bool
    RandomizeObject (uint32_t identifier, Type &out)
    {
        static std::unordered_map<uint32_t, Type> sm_Identifiers;
        sm_Identifiers.insert (std::make_pair (identifier, out));

        uint32_t model = sm_Identifiers[identifier];
        bool     ret   = RandomizeObject (model);

        out = model;
        return ret;
    }

    /*******************************************************/
    void
    AddSample (const Type value)
    {
    }
};

template <typename T, T... Values> class ConstantValues
{
    constexpr static std::array<T, sizeof...(Values)> m_Values{Values...};

public:
    using Type = T;

    void
    RandomizeObject (T &out) const
    {
        out = GetRandomElement (m_Values);
    }

    void
    AddSample (const T value)
    {
    }
};

/* A randomizer to shuffle a field's possible values */
template <typename T> class ShuffleRandomizer
{
    std::vector<T> Values;

public:
    using Type = T;

    void
    RandomizeObject (T &out) const
    {
        out = GetRandomElement (Values);
    }

    void
    AddSample (const T value)
    {
        Values.push_back (value);
    }
};

enum class SelectionType
{
    EXCLUDING,
    INCLUDING
};

/* A randomizer to only allow certain values through */
template <typename Randomizer, SelectionType type,
          typename Randomizer::Type... Values>
class SelectiveRandomizer
{
    Randomizer randomizer;

public:
    using Type = typename Randomizer::Type;

    bool
    IsValueAllowed (const Type value) const
    {
        if constexpr (type == SelectionType::EXCLUDING)
            {
                bool allowed = true;
                (..., (allowed = (value == Values) ? false : allowed));

                return allowed;
            }
        else
            {
                bool allowed = false;
                (..., (allowed = (value == Values) ? true : allowed));

                return allowed;
            }
    }

    void
    RandomizeObject (Type &out) const
    {
        if (IsValueAllowed (out))
            randomizer.RandomizeObject (out);
    }

    void
    AddSample (const Type value)
    {
        if (IsValueAllowed (value))
            randomizer.AddSample (value);
    }
};

/* Randomizer for atArray<T> */
template <typename Randomizer, typename T = atArray<typename Randomizer::Type>>
class ArrayRandomizer
{
    Randomizer randomizer;

public:
    using Type = T;

    void
    AddSample (const T &value)
    {
        for (const auto &elem : value)
            randomizer.AddSample (elem);
    }

    void
    RandomizeObject (T &out) const
    {
        for (auto &elem : out)
            randomizer.RandomizeObject (elem);
    }
};

template <typename Randomizer, uint32_t... Fields> class RandomizedFieldsWrapper
{
    std::array<Randomizer, sizeof...(Fields)> randomizers;

public:
    template <typename T>
    void
    AddSample (const T &data)
    {
        int i = 0;
        (..., randomizers[i++].AddSample (
                  data.template Get<typename Randomizer::Type> (Fields)));
    }

    template <typename T>
    void
    RandomizeObject (T &data) const
    {
        int i = 0;
        (..., randomizers[i++].RandomizeObject (
                  data.template Get<typename Randomizer::Type> (Fields)));
    }
};

template <typename T, typename... RandomizedFields> class ParserRandomHelper
{
    std::tuple<RandomizedFields...> fields;

public:
    using Type = T;

    void
    AddSample (const T &sample)
    {
        (..., std::get<RandomizedFields> (fields).AddSample (sample));
    }

    void
    RandomizeObject (T &object) const
    {
        (..., std::get<RandomizedFields> (fields).RandomizeObject (object));
    }
};

template <typename... Types> class ParserRandomHelperContainer
{
    std::tuple<Types...> randomizers;

    template <typename T>
    inline static bool
    CompareHash (const T &randomizer, uint32_t hash)
    {
        (void) randomizer;

        using ElemType = typename std::decay_t<T>::Type;
        return ElemType::GetHash () == hash
               || ElemType::GetLowercaseHash () == hash;
    }

    template <typename T, typename B>
    inline static void
    AddSample (T &randomizer, B *base, uint32_t hash)
    {
        using ElemType = typename std::decay_t<T>::Type;

        if (CompareHash (randomizer, hash))
            randomizer.AddSample (*reinterpret_cast<ElemType *> (base));
    }

    template <typename T, typename B>
    inline static void
    RandomizeObject (T &randomizer, B *base, uint32_t hash)
    {
        using ElemType = typename std::decay_t<T>::Type;

        if (CompareHash (randomizer, hash))
            randomizer.RandomizeObject (*reinterpret_cast<ElemType *> (base));
    }

public:
    template <typename T>
    void
    AddSample (T *base, uint32_t hash)
    {
        std::apply ([base,
                     hash] (auto &...x) { (..., AddSample (x, base, hash)); },
                    randomizers);
    }

    template <typename T>
    void
    RandomizeObject (T *base, uint32_t hash)
    {
        std::apply ([base, hash] (
                        auto &...x) { (..., RandomizeObject (x, base, hash)); },
                    randomizers);
    }
};

template <typename T, uint32_t FieldHash, typename FieldType = uint32_t>
class ParserRandomHelperContainerForEachFieldValue
{
    std::map<FieldType, T> Randomizers;

public:
    using Type = typename T::Type;

    T &
    GetRandomizerForObject (const Type &sample)
    {
        return Randomizers[sample.template Get<FieldType> (FieldHash)];
    }

    void
    AddSample (const Type &sample)
    {
        GetRandomizerForObject (sample).AddSample (sample);
    }

    void
    RandomizeObject (Type &object)
    {
        GetRandomizerForObject (object).RandomizeObject (object);
    }
};
