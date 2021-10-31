#pragma once

class ModCompatibility
{
    inline static bool sm_bIsInModScript = false;

    static bool &
    GetShouldRandomizeMods ()
    {
        static bool sm_bRandomizeMods = false;

        return sm_bRandomizeMods;
    }

public:
    static bool
    ShouldRandomize ()
    {
        return !sm_bIsInModScript || GetShouldRandomizeMods ();
    }

    bool
    GetInModScript ()
    {
        return sm_bIsInModScript;
    }

    struct InModScriptRAII
    {
        InModScriptRAII () { sm_bIsInModScript = true; }

        ~InModScriptRAII () { sm_bIsInModScript = false; }
    };

    ModCompatibility ();
};
