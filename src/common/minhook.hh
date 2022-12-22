#pragma once

#include <MinHook.h>
#include <string_view>
#include <utility>
#include <Utils.hh>

class MinHookWrapper
{
    static bool
    InitialiseMinHook ()
    {
        static bool s_Initialised = false;
        if (!std::exchange (s_Initialised, true))
            return MH_Initialize () == MH_STATUS::MH_OK;

        return true;
    }

public:
    template <typename O, typename F>
    static void
    HookBranchDestination (std::string_view pattern, uint32_t off, O &origFunc,
                           F hookedFunc, bool enable = true)
    {
        void *addr = hook::get_pattern (pattern, off);
        addr       = injector::GetBranchDestination (addr).get<void> ();

        RegisterHook (addr, origFunc, hookedFunc, enable);
    }

    template <typename O, typename F>
    static void
    RegisterHook (void *addr, O &origFunc, F hookedFunc, bool enable = true)
    {
        InitialiseMinHook ();
        MH_CreateHook (static_cast<LPVOID> (addr),
                       reinterpret_cast<LPVOID> (hookedFunc),
                       reinterpret_cast<LPVOID *> (&origFunc));

        if (enable)
            MH_EnableHook (MH_ALL_HOOKS);
    }

    template <typename O, typename F>
    static void
    RegisterHookOperand (const std::string &pattern, int32_t off, O &origFunc,
                         F hookedFunc, bool enable = true)
    {
        RegisterHook (GetRelativeReference (pattern, off, off + 4), origFunc,
                      hookedFunc, enable);
    }

    template <typename O, typename F>
    static void
    RegisterHookApi (const wchar_t *moduleName, const char *moduleProc,
                     O &origFunc, F hookedFunc, bool enable = true)
    {
        InitialiseMinHook ();

        MH_CreateHookApi (moduleName, moduleProc,
                          reinterpret_cast<LPVOID> (hookedFunc),
                          reinterpret_cast<LPVOID *> (&origFunc));

        if (enable)
            MH_EnableHook (MH_ALL_HOOKS);
    }
};

/*******************************************************/
#define REGISTER_MH_HOOK_API(modName, modProc, function, ret, ...)             \
    {                                                                          \
        static ret (*F) (__VA_ARGS__);                                         \
        MinHookWrapper::RegisterHookApi (L##modName, modProc, F, function<F>); \
    }

/*******************************************************/
#define REGISTER_MH_HOOK_BRANCH(pattern, off, function, ret, ...)              \
    {                                                                          \
        static ret (*F) (__VA_ARGS__);                                         \
        MinHookWrapper::HookBranchDestination (pattern, off, F, function<F>);  \
    }

/*******************************************************/
#define REGISTER_MH_HOOK_OPERAND(pattern, off, function, ret, ...)             \
    {                                                                          \
        static ret (*F) (__VA_ARGS__);                                         \
        MinHookWrapper::RegisterHookOperand (pattern, off, F, function<F>);    \
    }

/*******************************************************/
#define REGISTER_MH_HOOK(pattern, off, function, ret, ...)                     \
    {                                                                          \
        static ret (*F) (__VA_ARGS__);                                         \
        MinHookWrapper::RegisterHook (hook::get_pattern (pattern, off), F,     \
                                      function<F>);                            \
    }
