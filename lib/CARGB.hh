#pragma once

#include <cstdint>
struct CARGB
{
    union
    {
        struct
        {
            unsigned char b;
            unsigned char g;
            unsigned char r;
            unsigned char a;
        };
        int colour;
    };

    CARGB (unsigned char a, unsigned char r, unsigned char g, unsigned char b)
    {
        this->a = a;
        this->r = r;
        this->g = g;
        this->b = b;
    }

    CARGB () = default;

    bool
    operator== (const CARGB &rhs) const
    {
        return rhs.colour == this->colour;
    }
    bool
    operator!= (const CARGB &rhs) const
    {
        return !(rhs == *this);
    }
};

struct ColorFloat
{
    float r;
    float g;
    float b;

    ColorFloat () = default;

    ColorFloat (float r, float g, float b) : r (r), g (g), b (b) {}

    ColorFloat (const CARGB &argb)
    {
        r = argb.r / 255.0f;
        g = argb.g / 255.0f;
        b = argb.b / 255.0f;
    }

    CARGB
    ToARGB ()
    {
        return CARGB{255, static_cast<uint8_t> (r * 255),
                     static_cast<uint8_t> (g * 255),
                     static_cast<uint8_t> (b * 255)};
    }
};
