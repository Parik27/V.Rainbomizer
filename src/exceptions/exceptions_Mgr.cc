#include "exceptions_Mgr.hh"
#include <windows.h>
#include "common/logger.hh"

namespace Rainbomizer {

/*******************************************************/
ExceptionHandlerMgr &
ExceptionHandlerMgr::GetInstance ()
{
    static ExceptionHandlerMgr sm_Instance;
    return sm_Instance;
}

/*******************************************************/
void
ExceptionHandlerMgr::Init ()
{
    SetUnhandledExceptionFilter (RunExceptionHandler);
}

/*******************************************************/
ExceptionHandlerMgr::ExceptionHandlerMgr ()
{
    Logger::LogMessage ("Registering ExceptionHandlerMgr");
    
    atexit (RunExitHandler);
    SetUnhandledExceptionFilter (RunExceptionHandler);
}

/*******************************************************/
LONG CALLBACK
ExceptionHandlerMgr::RunExceptionHandler (_EXCEPTION_POINTERS *ep)
{
    RunExitHandler ();
    
    for (const auto &i : GetInstance().mExceptionHandlers)
        i->OnException(ep);
    
    return EXCEPTION_CONTINUE_SEARCH;
}

/*******************************************************/
void
ExceptionHandlerMgr::RunExitHandler ()
{
    for (const auto &i : GetInstance ().mExceptionHandlers)
        i->OnExit ();
}

}; // namespace Rainbomizer
