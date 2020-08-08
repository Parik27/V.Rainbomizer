#pragma once

#include <cstdint>

namespace rage {

constexpr char
NormaliseChar (const char c)
{
    if (c >= 'A' && c <= 'Z')
        return c + ('a' - 'A');

    else if (c == '\\')
        return '/';

    return c;
}

constexpr std::uint32_t
atPartialStringHash (char const *key, std::uint32_t initialHash = 0)
{
    uint32_t hash = initialHash;
    while (*key)
        {
            hash += NormaliseChar (*key++);
            hash += hash << 10;
            hash ^= hash >> 6;
        }
    return hash;
}

constexpr std::uint32_t
atLiteralStringHash (char const *s, size_t len)
{
    size_t   i    = 0;
    uint32_t hash = 0;
    while (i != len)
        {
            hash += s[i++];
            hash += hash << 10;
            hash ^= hash >> 6;
        }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

constexpr std::uint32_t
atStringHash (const char *s, std::uint32_t initialHash = 0)
{
    std::uint32_t hash = atPartialStringHash (s, initialHash);
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}
} // namespace rage

constexpr std::uint32_t operator"" _joaat (char const *s, size_t len)
{
    return rage::atLiteralStringHash (s, len);
}

#pragma pack(push, 1)
template <typename T = void> struct atArray
{
    T *      Data;
    uint16_t Size;
    uint16_t Capacity;
};

struct atString
{
    char *m_szString;
    short m_nLength;
    short m_nCapacity;
};
#pragma pack(pop)
