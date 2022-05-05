#pragma once

#include "exceptions_Mgr.hh"
#include <map>
#include <string>

namespace Rainbomizer {
/* Prints variables set by randomizers                      */
/*******************************************************/

class ExceptionHandler_Vars : public ExceptionHandler
{
    using VariableGroup = std::map<std::string, std::string>;
    inline static std::map<std::string, VariableGroup> m_Variables;

public:
    template <typename T>
    static void
    AddVariable (const std::string &ns, const std::string &name, const T &value)
    {
        if constexpr (std::is_arithmetic_v<T>)
            m_Variables[ns][name] = std::to_string (value);
        else
            m_Variables[ns][name] = value;
    }

    inline static void
    ClearNamespace (const std::string &ns)
    {
        m_Variables[ns].clear ();
    }

    void OnException (_EXCEPTION_POINTERS *ep) override;

    /*******************************************************/
    inline const char *
    GetHandlerName () override
    {
        return "Variables";
    }
};
}; // namespace Rainbomizer

class ExceptionVarsRecorder
{
    bool        m_ClearOnExit;
    std::string m_Namespace;

    using Vars = Rainbomizer::ExceptionHandler_Vars;

public:
    ExceptionVarsRecorder (const std::string &ns, bool clearOnExit = true)
        : m_Namespace (ns), m_ClearOnExit (clearOnExit)
    {
        Vars::AddVariable (m_Namespace, "In Scope", "yes");
    }

    ~ExceptionVarsRecorder ()
    {
        Vars::AddVariable (m_Namespace, "In Scope", "no");

        if (m_ClearOnExit)
            {
                Vars::ClearNamespace (m_Namespace);
            }
    }

    void
    Clear ()
    {
        Vars::ClearNamespace (m_Namespace);
    }

    template <typename T>
    void
    operator() (const std::string &key, const T &value)
    {
        Vars::AddVariable (m_Namespace, key, value);
    }
};
