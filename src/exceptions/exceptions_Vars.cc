#include "exceptions_Vars.hh"

#include <common/logger.hh>

using namespace Rainbomizer;

void
ExceptionHandler_Vars::OnException (_EXCEPTION_POINTERS *ep)
{
    Logger::LogMessage ("%s:", GetHandlerName ());
    Logger::LogMessage ("");

    for (auto &i : m_Variables)
        {
            Logger::LogMessage ("%s", i.first.c_str ());
            Logger::LogMessage ("=============================");
            for (auto &j : i.second)
                {
                    Logger::LogMessage ("\t%s = %s", j.first.c_str (),
                                        j.second.c_str ());
                }
            Logger::LogMessage ("=============================");
        }
}

REGISTER_HANDLER (ExceptionHandler_Vars)
