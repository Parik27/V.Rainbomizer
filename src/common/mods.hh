#pragma once

class ModCompatibility
{
    inline static bool sm_bIsInModScript = false;

public:
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
};
