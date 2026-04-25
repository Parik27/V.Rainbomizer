#include "Memory.hh"
#include "injector/injector.hpp"
#include "memory/GameAddress.hh"
#include "memory/MemorySignature.hh"
#include "memory/Pattern.hh"
#include <cstring>
#include <string_view>
#include <utility>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>

#include <Patterns/Patterns.hh>

#define GAME_INTERNAL_NAME "GTA3"

template <std::size_t NUM = 0>
inline void
InitialisePattern (MemoryManager &manager)
{
    using namespace MemorySignature;

    constexpr const auto &currentPattern = s_Patterns[NUM];

    if (GameAddress<currentPattern.address>::IsResolved ())
        return;

    uintptr_t addr = uintptr_t (
        hook::get_pattern (currentPattern.pattern_str));

    addr = currentPattern.resolver (addr);

    GameAddress<currentPattern.address>::SetResolvedAddress (addr);

    Rainbomizer::Logger::LogMessage (
        "[PATTERN] Resolved address %x to %x", currentPattern.address,
        GameAddress<currentPattern.address>::Get ());
}

std::string
MemoryManager::GetCacheFileName ()
{
    // Some random addresses that should hopefully be unique enough
    auto identifier = 0x10;

    return std::move ("SignatureCache_" + std::to_string (identifier) + ".bin");
}

void
MemoryManager::InitialiseAllPatterns ()
{
    // Rainbomizer::Logger::FunctionBenchmark benchmark;

    [this]<std::size_t... I> (std::index_sequence<I...>) {
        (..., InitialisePattern<I> (*this));
    }(std::make_index_sequence<std::size (s_Patterns)>{});
}

void
MemoryManager::Init ()
{
    if (std::exchange (m_initialised, true))
        return;

    InitialiseAllPatterns ();
}
