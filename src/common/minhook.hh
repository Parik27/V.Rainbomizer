#pragma once

#include <MinHook.h>
#include <string_view>
#include <utility>

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
