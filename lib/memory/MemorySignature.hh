#pragma once

#include <string_view>
#include <array>
#include <vector>
#include <cstdint>
#include <ranges>

/* Memory signature specification

   - Bytes separated by spaces ' '.
   - Bytes can be of the following format:
      - 1. The byte in hexadecimal. i.e AA, 01, A2
      - 2. One question mark '?' signifying the entire byte as a mask
      - 3. Two question marks, also signifying the entire byte as a mask
      - 4. Partial mask of the following format:
         - nibble mask: 1? A? - marks one of the nibbles as a mask
         - bitmask: Ghidra format: [.1..1001] square brackets with the byte in
   binary and '.' as masks.
*/

namespace MemorySignature {

inline constexpr uint8_t
ConvertHexDigit (char ch)
{
    if (ch >= 'A' && ch <= 'F')
        return uint8_t (ch - 'A' + 10);

    if (ch >= 'a' && ch <= 'f')
        return uint8_t (ch - 'a' + 10);

    return uint8_t (ch - '0');
}

template <typename T>
inline constexpr std::pair<uint8_t, uint8_t>
NibbleMaskFromString (T str)
{
    uint8_t byte = 0;
    uint8_t mask = 0;

    // ? or [A-Za-z0-9]
    if (str.size () == 1)
        {
            if (str[0] == '?')
                mask = 0xFF;
            else
                byte = ConvertHexDigit (str[0]);
        }

    // ?? or [A-Za-z0-9][A-Za-z0-9]
    else if (str.size () == 2)
        {
            if (str[0] == '?')
                mask |= 0b11110000;
            else
                byte |= ConvertHexDigit (str[0]) << 4;

            if (str[1] == '?')
                mask |= 0b1111;
            else
                byte |= ConvertHexDigit (str[1]);
        }

    // \[[.01]\]
    else if (str.size () == 10)
        {
            for (size_t i = 1; i < 9; i++)
                {
                    if (str[i] == '.' || str[i] == '?')
                        mask |= 1 << (8 - i);
                    else if (str[i] == '1')
                        byte |= 1 << (8 - i);
                }
        }

    return {byte, mask};
}

template <typename T>
consteval auto
GetMemorySignatureSize (T str)
{
    return std::ranges::distance (
        std::views::split (str, ' ')
        | std::views::filter ([] (const auto i) { return i.size () > 0; }));
}

template <uint32_t N> struct Signature
{
    std::array<uint8_t, N> bytes;
    std::array<uint8_t, N> masks;

    inline consteval Signature (std::string_view str)
    {
        constexpr char hexCharacters[]{'0', '1', '2', '3', '4', '5', '6', '7',
                                       '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        size_t idx = 0;
        for (const auto i : std::views::split (str, ' ')
                                | std::views::filter ([] (const auto i) {
                                      return i.size () > 0;
                                  }))
            {
                auto [byte, mask] = NibbleMaskFromString (i);
                bytes[idx]        = byte;
                masks[idx]        = mask;

                idx++;
            }
    }
};

}; // namespace MemorySignature
