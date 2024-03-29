#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <injector/calling.hpp>
#include <injector/hooking.hpp>

#include "Utils.hh"

#include <windows.h>
#include <cassert>

/* Hooking Utils for slightly more complicated (and very unsafe) hooking.
 * Potentially too large to merge into the actual Utils.hh, so I just split it
 * into this file. */

enum CallbackOrder
{
    CALLBACK_ORDER_BEFORE,
    CALLBACK_ORDER_AFTER
};

using _InstructionType = char[16];

template <uint32_t Addr, typename Ret, typename... Args> class ReplaceJmpHook
{
protected:
    static bool             mInitialised;
    static _InstructionType mStoredValue;
    static _InstructionType mOriginalValue;

    static _InstructionType *mHookedAddress;
    static uint32_t          mThreadId;

    inline static std::vector<std::function<bool (Args &...)>> &
    GetBeforeFunctions ()
    {
        static std::vector<std::function<bool (Args & ...)>> mBeforeFunctions;
        return mBeforeFunctions;
    }

    inline static auto &
    GetAfterFunctions ()
    {
        if constexpr (std::is_same_v<Ret, void>)
            {
                static std::vector<std::function<void (Args...)>>
                    mAfterFunctions;
                return mAfterFunctions;
            }
        else
            {
                static std::vector<std::function<void (Ret &, Args...)>>
                    mAfterFunctions;
                return mAfterFunctions;
            }
    }

public:
    virtual void Activate () = 0;

    static void
    SwapValues ()
    {
        std::swap (mStoredValue, *mHookedAddress);
    }

    static void
    Deactivate ()
    {
        std::swap (mOriginalValue, *mHookedAddress);
    }

    ReplaceJmpHook (void *addr, std::function<bool (Args &...)> callback)
    {
        mHookedAddress = (_InstructionType *) addr;
        GetBeforeFunctions ().push_back (callback);
    }

    ReplaceJmpHook (void *                                           addr,
                    std::decay_t<decltype (GetAfterFunctions ()[0])> callback)
    {
        mHookedAddress = (_InstructionType *) addr;
        GetAfterFunctions ().push_back (callback);
    }
};

template <uint32_t Addr, typename Ret, typename... Args>
class ReplaceJmpHook__fastcall : public ReplaceJmpHook<Addr, Ret, Args...>
{
    using base = ReplaceJmpHook<Addr, Ret, Args...>;

public:
    static Ret
    HookedFunction (Args... args)
    {
#ifndef NDEBUG
        assert (base::mThreadId == 0
                || base::mThreadId == GetCurrentThreadId ());
        base::mThreadId = GetCurrentThreadId ();
#endif

        bool inhibit = false;

        for (const auto &i : base::GetBeforeFunctions ())
            {
                inhibit = i (args...) ? inhibit : true;
            }

        if (inhibit)
            {
                if constexpr (std::is_same_v<Ret, void>)
                    {
                        return;
                    }
                else
                    {
                        return Ret ();
                    }
            }

        base::SwapValues ();

        if constexpr (!std::is_same_v<Ret, void>)
            {
                Ret ret = injector::fastcall<Ret (Args...)>::call (
                    base::mHookedAddress, args...);
                for (const auto &i : base::GetAfterFunctions ())
                    {
                        if constexpr (!std::is_same_v<Ret, void>)
                            {
                                i (ret, args...);
                            }
                    }

                base::SwapValues ();
                return ret;
            }
        else
            {
                injector::fastcall<Ret (Args...)>::call (base::mHookedAddress,
                                                         args...);
                for (const auto &i : base::GetAfterFunctions ())
                    {
                        if constexpr (!std::is_same_v<Ret, void>)
                            {
                                i (args...);
                            }
                    }
            }

        base::SwapValues ();
    }

    virtual void
    Activate () override
    {
        if (base::mInitialised)
            return;

        memcpy (base::mOriginalValue, *base::mHookedAddress,
                sizeof (_InstructionType));
        memcpy (base::mStoredValue, base::mOriginalValue,
                sizeof (_InstructionType));

        DWORD oldProtect = 0;
        injector::UnprotectMemory (base::mHookedAddress, 8, oldProtect);
        MakeJMP64 (base::mHookedAddress, HookedFunction);

        base::mInitialised = true;
    }

    using base::base;
};

#define INIT_VARIABLE(type, name, def)                                         \
    template <uint32_t Addr, typename Ret, typename... Args>                   \
    type ReplaceJmpHook<Addr, Ret, Args...>::name = def;

INIT_VARIABLE (bool, mInitialised, false)
INIT_VARIABLE (_InstructionType, mStoredValue, {0})
INIT_VARIABLE (_InstructionType, mOriginalValue, {0})
INIT_VARIABLE (_InstructionType *, mHookedAddress, 0)
INIT_VARIABLE (uint32_t, mThreadId, 0)

#undef INIT_VARIABLE

/*******************************************************/
class scoped_jmp64 : public injector::scoped_basic<12>
{
public:
    void
    make_jmp (injector::memory_pointer_tr at, injector::memory_pointer_raw dest,
              bool vp = true)
    {
        this->save (at, 12, vp);
        MakeJMP64 (at, dest);
    }

    // Constructors, move constructors, assigment operators........
    scoped_jmp64 ()                     = default;
    scoped_jmp64 (const scoped_jmp64 &) = delete;
    scoped_jmp64 (scoped_jmp64 &&rhs)
        : injector::scoped_basic<12> (std::move (rhs))
    {
    }
    scoped_jmp64 &operator= (const scoped_jmp64 &rhs) = delete;
    scoped_jmp64 &
    operator= (scoped_jmp64 &&rhs)
    {
        injector::scoped_basic<12>::operator= (std::move (rhs));
        return *this;
    }

    scoped_jmp64 (injector::memory_pointer_tr  at,
                  injector::memory_pointer_raw dest, bool vp = true)
    {
        make_jmp (at, dest, vp);
    }
};
