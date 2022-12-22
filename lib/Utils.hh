#pragma once

#include "Patterns/Patterns.hh"
#include "ModUtils/Trampoline.h"
#include "injector/injector.hpp"
#include <cstdint>
#include <vector>
#include <utility>
#include <optional>
#include <algorithm>

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

int          RandomInt (int max);
int          RandomInt (int min, int max);
size_t       RandomSize (size_t max);
float        RandomFloat (float min, float max);
float        RandomFloat (float max);
unsigned int RandomWeighed (const std::vector<double> &weights);
bool         RandomBool (float Odds, float Precision = 1.0f);

void InitialiseAllComponents ();

/*******************************************************/
/* Returns a random element from a container           */
/*******************************************************/
template <typename T>
auto &
GetRandomElement (const T &container)
{
    auto it = std::begin (container);
    std::advance (it, RandomSize (std::size (container) - 1));

    return *it;
}

template <typename T>
auto &
GetRandomElementMut (T &container)
{
    auto it = std::begin (container);
    std::advance (it, RandomSize (std::size (container) - 1));

    return *it;
}

template <typename Map, typename Key>
auto
LookupMap (Map &m, const Key &k)
{
    auto it = m.find (k);
    return it != m.end () ? &it->second : nullptr;
}

/*******************************************************/
template <typename T, typename C, typename O>
inline T *
GetAtOffset (C *classInst, O offset)
{
    return (T *) (reinterpret_cast<char *> (classInst) + offset);
}

/*******************************************************/
template <typename T>
std::pair<void *, const T &>
GetPatternsAndData (
    std::vector<std::tuple<std::string, int, const T &>> patterns)
{
    for (auto &[pattern, offset, data] : patterns)
        {
            hook::pattern p (pattern);
            if (p.size ())
                return std::pair (p.get_first (offset), data);
        }

    return std::pair (nullptr, T ());
}

/*******************************************************/
inline void *
GetPatterns (std::vector<std::pair<std::string, int>> patterns)
{
    for (auto &[pattern, offset] : patterns)
        {
            hook::pattern p (pattern);
            if (p.size ())
                return p.get_first (offset);
        }

    return nullptr;
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
template <bool Jmp = false, typename F>
void
RegisterHook (void *addr, F hookedFunc)
{
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
template <bool Jmp = false, typename F, typename O>
void
RegisterHook (void *addr, O &originalFunc, F hookedFunc)
{
    ReadCall (addr, originalFunc);
    RegisterHook<Jmp> (addr, hookedFunc);
}

/*******************************************************/
template <uint32_t offset, typename F, typename O>
void
RegisterHookVft (void *vft, O &originalFunc, F hookedFunc)
{
    void *&address = reinterpret_cast<void **> (vft)[offset];

    injector::WriteMemory (&originalFunc, address);
    injector::WriteMemory (&address, hookedFunc);
}

/*******************************************************/
template <typename T, uint32_t offset, typename F, typename O>
void
RegisterHookVft (O &originalFunc, F hookedFunc)
{
    RegisterHookVft<offset> ((void *) T::vftable, originalFunc, hookedFunc);
}

/*******************************************************/
template <bool Jmp = false, typename F, typename O>
void
RegisterHook (const std::string &pattern, int offset, O &originalFunc,
              F hookedFunc)
{
    RegisterHook<Jmp> (hook::get_pattern (pattern, offset), originalFunc,
                       hookedFunc);
}

/*******************************************************/
template <bool Jmp = false, typename F>
void
RegisterHook (const std::string &pattern, int offset, F hookedFunc)
{
    void *addr = hook::get_pattern (pattern, offset);
    RegisterHook<Jmp> (addr, hookedFunc);
}

/*******************************************************/
template<typename F, typename O>
void
RegisterHookOperand (const std::string &pattern, int offset, F hookedFunc, O& originalFunc)
{
    hook::pattern p (pattern);
    auto result = p.get_one ();

    injector::WriteMemory (originalFunc, result.get<char> (offset + 4)
                                             + *result.get<int32_t> (offset));

    *result.get<int32_t> (offset)
        = Trampoline::MakeTrampoline (GetModuleHandle (nullptr))
        ->Jump (hookedFunc) - result.get<char>(offset + 4);
}

/*******************************************************/
void RegisterJmpHook (void *addr, void *dst, void **outOrignal, int size);

template <int size, typename F, typename O>
inline void
RegisterJmpHook (void *addr, O &originalFunc, F hookedFunc)
{
    static_assert (
        size >= 12,
        "Size should be greater than the JMP instruction (12 bytes)");

    RegisterJmpHook (addr, (void *) hookedFunc, (void **) &originalFunc, size);
}

template <int size, typename F, typename O>
inline void
RegisterJmpHook (const std::string &pattern, int offset, O &originalFunc,
                 F hookedFunc)
{
    RegisterJmpHook<size> (hook::get_pattern (pattern, offset), originalFunc,
                           hookedFunc);
}

/*******************************************************/
void MakeJMP64 (injector::memory_pointer_tr  at,
                injector::memory_pointer_raw dest);

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
    int32_t  offset = *(int32_t *) (addr + dataOffset);
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

/*******************************************************/
/* Macro to facilitate a hook
   Define a function like this:

   template<auto &O>
   Ret HookedFunction (...)
   {
   }

   Then register the hook like
   REGISTER_HOOK (Pattern, Offset, HookedFunction, Signature);
*/
/*******************************************************/
#define REGISTER_HOOK(pattern, offset, function, ret, ...)                     \
    {                                                                          \
        static ret (*F) (__VA_ARGS__);                                         \
        RegisterHook (pattern, offset, F, function<F>);                        \
    }

#define REGISTER_HOOK_OPERAND(pattern, offset, function, ret, ...)             \
    {                                                                          \
        static ret (*F) (__VA_ARGS__);                                         \
        RegisterHookOperand (pattern, offset, F, function<F>);                 \
    }

/*******************************************************/
#define REGISTER_HOOK_JMP(pattern, offset, function, ret, ...)                 \
    {                                                                          \
        static ret (*F) (__VA_ARGS__);                                         \
        RegisterHook<true> (pattern, offset, F, function<F>);                  \
    }

/*******************************************************/
#define REGISTER_JMP_HOOK(size, pattern, offset, function, ret, ...)           \
    {                                                                          \
        static ret (*F) (__VA_ARGS__);                                         \
        RegisterJmpHook<size> (pattern, offset, F, function<F>);               \
    }

/*******************************************************/
#define REGISTER_VFT_HOOK(type, offset, function, ret, ...)                    \
    {                                                                          \
        static ret (*F) (__VA_ARGS__);                                         \
        RegisterHookVft<type, offset> (F, function<F>);                        \
    }
