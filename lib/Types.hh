#pragma once

class SpecialBool
{
    bool check = false;

public:
    operator bool () { return check; }

    void
    Reset ()
    {
        check = false;
    }

    bool
    operator= (bool other)
    {
        if (other)
            check = true;

        return other;
    }
};
