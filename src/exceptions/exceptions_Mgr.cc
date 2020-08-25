#include "exceptions_Mgr.hh"
#include <windows.h>
#include "common/logger.hh"
#include <thread>

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
ExceptionHandlerMgr::RunThread ()
{
    while (true)
        {
            if (GetAsyncKeyState (VK_F8) && GetAsyncKeyState (VK_F9)
                && GetAsyncKeyState (VK_F10))
                {
                    ExceptionHandlerMgr::GetInstance ().RunExitHandler ();
                    abort ();
                }
            
            using namespace std::chrono_literals;
            std::this_thread::sleep_for (1s);
        }
}

/*******************************************************/
void
ExceptionHandlerMgr::Init ()
{
    SetUnhandledExceptionFilter (RunExceptionHandler);

    static std::thread handlerThread (this->RunThread);
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
