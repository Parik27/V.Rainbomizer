#include <cstdint>

namespace rage {
constexpr std::uint32_t
atStringHash (char const *s, size_t len)
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

std::uint32_t atStringHashLowercase (const char *s);
} // namespace rage

constexpr std::uint32_t operator"" _joaat (char const *s, size_t len)
{
  return rage::atStringHash (s, len);
}

#pragma pack(push, 1)
template<typename T = void>
struct
atArray
{
    T* Data;
    uint16_t Size;
    uint16_t Capacity;
};

struct atString {
    char * m_szString;
    short m_nLength;
    short m_nCapacity;
};
#pragma pack(pop)
