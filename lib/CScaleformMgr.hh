#pragma once

#include "rage.hh"
#include <cstdint>

struct ScaleformMethodStruct
{

    struct ParamStruct
    {
        atString *Name;
        uint32_t  Type;
        void *    Data;

        double *
        GetDouble ()
        {
            if (Type == 3)
                return static_cast<double *> (Data);
            return nullptr;
        }

        bool *
        GetBool ()
        {
            if (Type == 2)
                return static_cast<bool *> (Data);
            return nullptr;
        }

        char **
        GetStr ()
        {
            if (Type == 4)
                return reinterpret_cast<char **>(&Data);
            return nullptr;
        }

        explicit operator bool () { return Type; }
    };
};
