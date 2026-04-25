#pragma once

#include "mission/missions_YscUtils.hh"
#include <vector>

class YscCodeEdits
{
    using CodeEditFunction = bool (*) (YscUtilsOps &);

    struct CodeEdit
    {
        std::string      Name;
        CodeEditFunction Function;
        bool             Enabled;
    };

    static std::vector<CodeEdit> &
    GetEdits ()
    {
        static std::vector<CodeEdit> sm_Edits;
        return sm_Edits;
    }

    /*******************************************************/
    template <auto &scrProgram_InitNativeTablese188>
    static bool
    ApplyCodeFixes (scrProgram *program)
    {
        YscUtilsOps utils (program);
        bool        ret = scrProgram_InitNativeTablese188 (program);

        for (const auto &i : GetEdits ())
            {
                if (!i.Enabled)
                    continue;

                if (!i.Function (utils))
                    continue;

                Rainbomizer::Logger::LogMessage (
                    "[YscCodeEdits] Applying Code Edit (%s): %s",
                    i.Name.c_str (), utils ? "Failed" : "Succeeded");
            }

        return ret;
    }

public:
    static void
    Add (std::string_view name, CodeEditFunction f, bool enabled = true)
    {
        Initialise ();
        GetEdits ().push_back ({std::string (name), f, enabled});
    }

    static void
    Initialise ()
    {
        static bool sm_Initialised = false;
        if (!std::exchange (sm_Initialised, true))
            REGISTER_HOOK (100035, ApplyCodeFixes, bool, scrProgram *);
    }

    friend class CodeEditDebugInterface;
};
