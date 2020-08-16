#pragma once

#include <vector>
#include <functional>
#include <memory>

struct _EXCEPTION_POINTERS;

namespace Rainbomizer {

/*******************************************************/
class ExceptionHandler
{
public:
    virtual void OnExit (){}; // Also called OnException
    virtual void OnException (_EXCEPTION_POINTERS *){};

    virtual const char *GetHandlerName () = 0;

    virtual ~ExceptionHandler() = default;
};

/*******************************************************/
/* Class to handle and log crashes/exceptions when they happen */
/*******************************************************/
class ExceptionHandlerMgr
{
    std::vector<std::unique_ptr<ExceptionHandler>> mExceptionHandlers;

    static long __stdcall RunExceptionHandler (_EXCEPTION_POINTERS *ep);
    static void RunExitHandler ();

public:
    ExceptionHandlerMgr ();
    ExceptionHandlerMgr (const ExceptionHandlerMgr &) = delete;

    void Init ();

    static void RunThread ();
    
    static ExceptionHandlerMgr &GetInstance ();

    template <typename T, typename... Args>
    static T *
    RegisterHandler (Args... args)
    {
        GetInstance ().mExceptionHandlers.push_back (
            std::make_unique<T> (args...));
        return static_cast<T *> (
            GetInstance ().mExceptionHandlers.back ().get ());
    }
};

#define REGISTER_HANDLER(className)                                            \
    namespace ExceptionHandlers {                                              \
    auto className##_inst                                                      \
        = ExceptionHandlerMgr::RegisterHandler<className> ();                  \
    }

} // namespace Rainbomizer
