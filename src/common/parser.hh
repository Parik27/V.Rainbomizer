#pragma once

#include "ParserUtils.hh"
#include <type_traits>
#include <tuple>
#include <array>

/* A randomizer to randomize a field between its min/max value */
template <typename T> class RangedRandomizer
{
    T Min;
    T Max;

public:

    using Type = T;
    
    void
    Randomize (T& out) const
    {
        out = RandomFloat (Min, Max);
    }

    void
    AddValue (const T value)
    {
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
    Randomize (T& out) const
    {
        out = GetRandomElement (Values);
    }

    void
    AddValue (const T value)
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
    Randomize (Type &out) const
    {
        if (IsValueAllowed (out))
            randomizer.Randomize (out);
    }

    void
    AddValue (const Type value)
    {
        if (IsValueAllowed (value))
            randomizer.AddValue (value);
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
    AddValue (const T &value)
    {
        for (const auto &elem : value)
            randomizer.AddValue (elem);
    }

    void
    Randomize (T &out) const
    {
        for (auto &elem : out)
            randomizer.Randomize (elem);
    }
};

template <typename Randomizer, uint32_t... Fields> class RandomizedFieldsWrapper
{
    std::array<Randomizer, sizeof...(Fields)> randomizers;

public:
    template <typename T>
    void
    AddValue (const T &data)
    {
        int i = 0;
        (..., randomizers[i++].AddValue (
                  data.template Get<typename Randomizer::Type> (Fields)));
    }

    template <typename T>
    void
    Randomize (T &data) const
    {
        int i = 0;
        (..., randomizers[i++].Randomize (
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
        (..., std::get<RandomizedFields> (fields).AddValue (sample));
    }

    void
    RandomizeObject (T &object) const
    {
        (..., std::get<RandomizedFields> (fields).Randomize (object));
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
        return ElemType::GetHash () == hash;
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
    RandomizeObject (const T &randomizer, B *base, uint32_t hash)
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
