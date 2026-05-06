#pragma once

// define this if you want to get logs if a pattern is used before
// initialisation
// #define DEBUG_GAME_ADDRESSES

#include "Utils.hh"
#include "common/logger.hh"
#include "injector/injector.hpp"
#include "memory/Pattern.hh"
#ifdef DEBUG_GAME_ADDRESSES
#include <utility>
#include <core/Logger.hh>
#endif

#include <cstdint>

/* A class that will store the addresses after pattern resolution and contains
 * helper function to interact with the said addresses */
template <uintptr_t Address> class GameAddress
{
    inline static uintptr_t resolvedAddress = Address;
    inline static bool      resolved        = false;

#ifdef DEBUG_GAME_ADDRESSES
    inline static bool unresolvedMessagePrinted = false;
#endif

public:
    static uintptr_t
    Get ()
    {
        (void)PatternTracker<Address>::entry;

#ifdef DEBUG_GAME_ADDRESSES
        if (!resolved && !std::exchange (unresolvedMessagePrinted, true))
            Rainbomizer::Logger::LogMessage (
                "Address %08x used before resolution", Address);
#endif
        return resolvedAddress;
    }

    template<typename T>
    static T
    GetAs ()
    {
        return reinterpret_cast<T>(resolvedAddress);
    }

    template <typename T>
    static T &
    Read ()
    {
        return *reinterpret_cast<T *> (Get ());
    }

    template <typename T>
    static void
    Write (const T &value)
    {
        Read<T> () = value;
    }

    static bool
    IsResolved ()
    {
        return resolved;
    }

    static const uintptr_t &
    GetResolvedAddress ()
    {
        return resolvedAddress;
    }

    static void
    SetResolvedAddress (uintptr_t addr)
    {
        if (addr != 0)
            {
                resolvedAddress = addr;
                resolved        = true;

                Rainbomizer::Logger::LogMessage (
                    "GameAddress %d resolved to %p", Address, resolvedAddress);
            }
    }

    template <bool Check = true>
    static void
    Nop (size_t size = 0)
    {
        if constexpr (Check)
            {
                if (!IsResolved ())
                    return;
            }

        injector::MakeNOP (Get (), size);
    }

    template <typename T, bool Check = true>
    static void
    WriteMemory (T value)
    {
        if constexpr (Check)
            {
                if (!IsResolved ())
                    return;
            }

        injector::WriteMemory (Get (), value);
    }

    template <bool Check = true>
    static void
    WriteMemoryRaw (void* value, size_t size, bool vp = true)
    {
        if constexpr (Check)
            {
                if (!IsResolved ())
                    return;
            }

        injector::WriteMemoryRaw (Get (), value, size, vp);
    }
    GameAddress () = delete;
};

template <uintptr_t Address, typename Prototype> class GameFunction;

template <uintptr_t Address, typename Ret, typename... Args>
class GameFunction<Address, Ret (Args...)>
{
public:
    inline static Ret
    Call (Args... args)
    {
        return ((Ret (*) (Args...)) (GameAddress<Address>::Get ())) (args...);
    }

    inline static Ret
    operator() (Args... args)
    {
        return Call (args...);
    }

    auto
    operator& ()
    {
        return GameAddress<Address>::template GetAs <Ret (*) (Args...)> ();
    }
};

template <typename T, uintptr_t Address> class GameVariable
{
public:
    static T &
    Get ()
    {
        return GameAddress<Address>::template Read<T> ();
    }

    operator T &() const { return GameAddress<Address>::template Read<T> (); }

    auto
    operator& ()
    {
        return GameAddress<Address>::template GetAs <T*> ();
    }

    auto
    operator->()
    {
        if constexpr (std::is_pointer_v<T>)
            return Get ();
        else
            return &Get ();
    }
};

template <uintptr_t Address, typename OffsetType = int32_t> class GameOffset
{
    inline static GameVariable<OffsetType, Address> offsetVar{};

public:
    template <typename ReturnType, typename ClassType>
    static ReturnType &
    Get (ClassType *cls)
    {
        return *GetAtOffset<ReturnType> (cls, offsetVar.Get ());
    }
};

#define GAMEADDR(x) GameAddress<(uintptr_t) x>::Get ()
#define GAME_ADDR_WRAPPER template <int UNUSED_EVAL_PARAM = 0>
