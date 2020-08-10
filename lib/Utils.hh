#pragma once

#include "Patterns/Patterns.hh"
#include "injector/hooking.hpp"
#include "ModUtils/Trampoline.h"
#include <cstdint>

template <typename Func, typename Addr>
void
ReadCall (Addr address, Func &func)
{
    func = (Func) injector::GetBranchDestination (address).as_int ();
}

template <typename Func, typename Addr>
void
ConvertCall (Addr address, Func &func)
{
    func = Func (address);
}

int  RandomInt (int max);
int  RandomInt (int min, int max);
float RandomFloat (float min, float max);
float RandomFloat (float max);
void InitialiseAllComponents ();

/*******************************************************/
/* Returns a random element from a container           */
/*******************************************************/
template<typename T>
auto&
GetRandomElement (const T& container)
{
    auto it = std::begin(container);
    std::advance (it, RandomInt (std::size(container) - 1));

    return *it;
}

/*******************************************************/
template <typename T, typename V>
bool
DoesElementExist (const T &container, const V val)
{
    return std::find (std::begin (container), std::end (container), val)
           != std::end (container);
}

/*******************************************************/
template <bool Jmp = false, typename F, typename O>
void
RegisterHook (const std::string &pattern, int offset, O &originalFunc,
              F hookedFunc)
{
    void *addr = hook::get_pattern (pattern, offset);
    ReadCall (addr, originalFunc);
    if constexpr (Jmp)
        injector::MakeJMP (addr, Trampoline::MakeTrampoline (
                                      GetModuleHandle (nullptr))
                                      ->Jump (hookedFunc));
    else
        injector::MakeCALL (addr, Trampoline::MakeTrampoline (
                                     GetModuleHandle (nullptr))
                                     ->Jump (hookedFunc));
}

/*******************************************************/
template <bool Jmp = false, typename F>
void
RegisterHook (const std::string &pattern, int offset, F hookedFunc)
{
    void *addr = hook::get_pattern (pattern, offset);
    if constexpr (Jmp)
        injector::MakeJMP (addr, Trampoline::MakeTrampoline (
                                      GetModuleHandle (nullptr))
                                      ->Jump (hookedFunc));
    else
        injector::MakeCALL (addr, Trampoline::MakeTrampoline (
                                     GetModuleHandle (nullptr))
                                     ->Jump (hookedFunc));
}

/*******************************************************/
void
MakeJMP64 (injector::memory_pointer_tr at, injector::memory_pointer_raw dest);

/*******************************************************/
template <typename T = void>
void *
SearchBack (const std::string &pattern, const std::string &pattern2,
            int max_offset, int offset = 0)
{
    puts (pattern.c_str ());
    injector::memory_pointer_raw addr = hook::get_pattern (pattern);

    return hook::make_range_pattern ((addr - max_offset).as_int (),
                                     addr.as_int (), pattern2)
        .get_one ()
        .get<T> (offset);
}

/*******************************************************/
template <typename T = void>
T *
GetRelativeReference (const std::string &pattern, int dataOffset,
                      int nextInstOffset)
{
    uint8_t *addr   = hook::get_pattern<uint8_t> (pattern);
    int32_t offset = *(int32_t *) (addr + dataOffset);
    return (T *) (addr + offset + nextInstOffset);
}

/*******************************************************/
template <typename T = void>
T *
GetRelativeReference (const std::string &pattern, int dataOffset)
{
    uint32_t offset = *hook::get_pattern<uint32_t> (pattern, dataOffset);
    return (T *) (hook::getRVA (offset));
}
