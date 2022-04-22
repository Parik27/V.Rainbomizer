#pragma once

#include <deque>
#include <cstdint>
#include <string.h>
#include <string_view>
#include <type_traits>

#include "atArray.hh"

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
atPartialStringHash (std::string_view key, std::uint32_t initialHash = 0)
{
    uint32_t hash = initialHash;
    for (auto c : key)
        {
            hash += NormaliseChar (c);
            hash += hash << 10;
            hash ^= hash >> 6;
        }
    return hash;
}

constexpr std::uint32_t
atLiteralStringHash (std::string_view key, std::uint32_t initialHash = 0)
{
    uint32_t hash = initialHash;
    for (auto c : key)
        {
            hash += c;
            hash += hash << 10;
            hash ^= hash >> 6;
        }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

constexpr std::uint32_t
atStringHash (std::string_view key, std::uint32_t initialHash = 0)
{
    std::uint32_t hash = atPartialStringHash (key, initialHash);
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

class float16
{
public:
    uint16_t value;

    float16 () : value (0){};
    float16 (uint16_t val) : value (val){};
    float16 (float val) : value (from_float (val).value){};

    float
    to_float ()
    {
        uint32_t f = ((value & 0x8000) << 16)
                     | (((value & 0x7c00) + 0x1C000) << 13)
                     | ((value & 0x03FF) << 13);
        return *reinterpret_cast<float *> (&f);
    }

    inline static float16
    from_float (float f)
    {
        uint32_t x = *reinterpret_cast<uint32_t *> (&f);
        uint16_t h = static_cast<uint16_t> (
            ((x >> 16) & 0x8000)
            | ((((x & 0x7f800000) - 0x38000000) >> 13) & 0x7c00)
            | ((x >> 13) & 0x03ff));
        return h;
    }
};

static_assert (sizeof (float16) == 2, "size of half not 2 bytes");

template <int size> class bitset
{
    uint32_t m_nBits[size / 32];

public:
    bool
    operator[] (size_t pos) const
    {
        return m_nBits[pos / 32] & (1 << (pos % 32));
    }

    inline void
    Set (size_t pos, bool value)
    {
        uint32_t &bits = m_nBits[pos / 32];
        if (value)
            bits = bits | (1 << (pos % 32));
        else
            bits = bits & ~(1 << (pos % 32));
    }
};

class Vec2f
{
public:
    float x;
    float y;
};

class Vec2V
{
public:
    float x;
    float y;

private:
    float __pad[2];
};

class Vec3f
{
public:
    float x;
    float y;
    float z;
};

class Vec3V
{
public:
    float x;
    float y;
    float z;

private:
    float __pad;
};

class Vec4V
{
public:
    float x;
    float y;
    float z;
    float w;
};

class Mat33V
{
public:
    Vec3V right;
    Vec3V up;
    Vec3V at;
};

class Mat34V
{
public:
    Vec3V right;
    Vec3V up;
    Vec3V at;
    Vec3V pos;
};

class Mat44V
{
public:
    Vec4V right;
    Vec4V up;
    Vec4V at;
    Vec4V pos;
};

} // namespace rage

/* Not a part of rage */
template <typename T, int capacity> class CyclicContainer
{
    std::deque<T> m_Internal{};
    bool          m_Full = false;

public:
    void
    Push (const T &value)
    {
        if (m_Full)
            {
                m_Internal.pop_front ();
            }
        m_Internal.push_back (value);
        m_Full = m_Internal.size () >= capacity;
    }

    inline const std::deque<T>
    Get () const
    {
        return m_Internal;
    }
};

constexpr std::uint32_t operator"" _joaat (char const *s, size_t len)
{
    return rage::atLiteralStringHash (std::string_view (s, len), 0);
}

#pragma pack(push, 1)

struct atString
{
    char *m_szString;
    unsigned short m_nLength;
    unsigned short m_nCapacity;

    atString () = default;

    atString (const char *str)
    {
        m_szString  = const_cast<char *> (str);
        m_nLength   = static_cast<unsigned short> (strlen (str));
        m_nCapacity = m_nLength;
    }
};
#pragma pack(pop)
