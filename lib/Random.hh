#pragma once

#include <iterator>
#include <random>
#include <ctime>

template <typename Engine = std::mt19937> class RandomUtils
{
    Engine m_Engine{(unsigned int) time (NULL)};

public:
    Engine &
    GetEngine ()
    {
        return m_Engine;
    }

    template <typename T>
    T
    Get (T min, T max)
    {
        if constexpr (std::is_integral_v<T>)
            {
                std::uniform_int_distribution<T> d{min, max};
                return d (GetEngine ());
            }
        else
            {
                std::uniform_real_distribution<T> d{min, max};
                return d (GetEngine ());
            }
    }

    template <typename T>
    T
    Get (T max)
    {
        return Get<T> (0, max);
    }

    template <typename T>
    T
    GetWeighed (const std::vector<double> &weights)
    {
        std::discrete_distribution<T> dist{weights.begin (), weights.end ()};
        return dist (GetEngine ());
    }

    template <typename T>
    auto &
    Element (const T &container)
    {
        auto it = std::begin (container);
        std::advance (it, Get (std::size (container) - 1));

        return *it;
    }

    template <typename T>
    auto &
    ElementMut (T &container)
    {
        auto it = std::begin (container);
        std::advance (it, Get (std::size (container) - 1));

        return *it;
    }
};
