#pragma once

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
