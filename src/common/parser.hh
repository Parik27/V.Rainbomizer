#pragma once

#include "ParserUtils.hh"
#include <cstdint>
#include <map>
#include <type_traits>
#include <tuple>
#include <array>

/* A randomizer to randomize a field between its min/max value */
template <typename T> class RangedRandomizer
{
    bool bInitialised = false;
    
    T Min;
    T Max;

public:

    using Type = T;
    
    void
    RandomizeObject (T& out) const
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

/* A randomizer to shuffle a field's possible values */
template <typename T> class ShuffleRandomizer
{
    std::vector<T> Values;

public:

    using Type = T;
    
    void
    RandomizeObject (T& out) const
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
template <typename Randomizer, SelectionType type, typename Randomizer::Type... Values>
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
template <typename Randomizer,
          typename T = atArray<typename Randomizer::Type>>
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
        using ElemType = typename std::decay_t<T>::Type;
        return ElemType::GetHash () == hash || ElemType::GetLowercaseHash () == hash;
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
                     hash] (auto &... x) { (..., AddSample (x, base, hash)); },
                    randomizers);
    }

    template <typename T>
    void
    RandomizeObject (T *base, uint32_t hash)
    {
        std::apply (
            [base, hash] (auto &... x) {
                (..., RandomizeObject (x, base, hash));
            },
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
