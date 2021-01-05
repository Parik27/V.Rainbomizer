#pragma once

#include <deque>
#include <cstdint>
#include <string_view>
#include <type_traits>

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

// It isn't actually in the rage:: namespace, but I put it here to prevent
// conflicts with other classes that might be called half.
class half
{
public:
    uint16_t value;

    half () : value (0){};
    half (uint16_t val) : value (val){};
    half (const half &other) : value (other.value){};
    half (float val) : value (from_float (val).value){};

    float
    to_float ()
    {
        uint32_t f = ((value & 0x8000) << 16)
                     | (((value & 0x7c00) + 0x1C000) << 13)
                     | ((value & 0x03FF) << 13);
        return *(float *) &f;
    }

    inline static half
    from_float (float f)
    {
        uint32_t x = *((uint32_t *) &f);
        uint16_t h = ((x >> 16) & 0x8000)
                     | ((((x & 0x7f800000) - 0x38000000) >> 13) & 0x7c00)
                     | ((x >> 13) & 0x03ff);
        return h;
    }
};

static_assert (sizeof (half) == 2, "size of half not 2 bytes");

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
} // namespace rage

/* Not a part of rage */
template <typename T, int capacity> class CyclicContainer
{
    std::deque<T> m_Internal{};
    bool m_Full = false;

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

template <typename T> struct atArrayBase
{
    T Data;

    using ElemType = std::decay_t<decltype (Data[0])>;

    ElemType &
    operator[] (size_t ix)
    {
        return Data[ix];
    }

    const ElemType &
    operator[] (size_t ix) const
    {
        return Data[ix];
    }

    constexpr ElemType *
    begin ()
    {
        return &Data[0];
    }

    constexpr const ElemType *
    begin () const
    {
        return &(*this)[0];
    }
};

template <typename T> struct atArrayGetSizeWrapperObject
{
    T *Data;

    T &
    operator[] (size_t ix)
    {
        return *reinterpret_cast<T *> (reinterpret_cast<char *> (Data)
                                       + T::GetSize () * ix);
    }

    const T &
    operator[] (size_t ix) const
    {
        return *reinterpret_cast<T *> (reinterpret_cast<char *> (Data)
                                       + T::GetSize () * ix);
    }
};

template <typename T = void *>
struct atArrayGetSizeWrapper
    : public atArrayBase<atArrayGetSizeWrapperObject<T>>
{
    uint16_t Size;
    uint16_t Capacity;

    constexpr T *
    end ()
    {
        return &(*this)[Size];
    }

    constexpr const T *
    end () const
    {
        return &(*this)[Size];
    }
};

template <typename T = void *> struct atArray : public atArrayBase<T *>
{
    uint16_t Size;
    uint16_t Capacity;

    constexpr T *
    end ()
    {
        return &(*this)[Size];
    }

    constexpr const T *
    end () const
    {
        return &(*this)[Size];
    }
};

template <typename T, uint32_t Size>
struct atFixedArray : public atArrayBase<T[Size]>
{
    constexpr T *
    end ()
    {
        return &(*this)[Size];
    }

    constexpr const T *
    end () const
    {
        return &(*this)[Size];
    }
};

struct atString
{
    char *m_szString;
    short m_nLength;
    short m_nCapacity;
};
#pragma pack(pop)
