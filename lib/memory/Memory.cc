#include "Memory.hh"
#include "GameVersion.hh"
#include "common/common.hh"
#include "injector/injector.hpp"
#include "memory/GameAddress.hh"
#include "memory/MemorySignature.hh"
#include "memory/Pattern.hh"
#include "memory/RuntimePattern.hh"
#include <cstring>
#include <string_view>
#include <utility>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>

#include <Patterns/Patterns.hh>

#define GAME_INTERNAL_NAME "GTA3"

void ResolveGameAddresses (const Rainbomizer::RuntimePatternManager &manager);

template <std::size_t NUM = 0>
inline void
InitialisePattern (MemoryManager &manager)
{
    using namespace MemorySignature;

    constexpr const auto &currentPattern = s_Patterns[NUM];

    if (GameAddress<currentPattern.address>::IsResolved ())
        return;

    auto pattern = hook::pattern (currentPattern.pattern_str);
    if (pattern.size () < currentPattern.matchIdx + 1)
        {
            Rainbomizer::Logger::LogMessage (
                "[PATTERN] Failed to find pattern for address %x",
                currentPattern.address);
            return;
        }

    uintptr_t addr
        = uintptr_t (pattern.get (currentPattern.matchIdx).get<void *> ());

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

    FILE *patternsFile;

    if (GameVersion::IsEnhanced ())
        patternsFile = Rainbomizer::Common::GetRainbomizerDataFile (
            "patterns_enhanced.bin");
    else
        patternsFile = Rainbomizer::Common::GetRainbomizerDataFile (
            "patterns_legacy.bin");

    Rainbomizer::RuntimePatternManager manager;
    manager.ReadFromFile (patternsFile);
    manager.ResolvePatterns ();

    ResolveGameAddresses (manager);

    if constexpr (false)
        {
            [this]<std::size_t... I> (std::index_sequence<I...>) {
                (..., InitialisePattern<I> (*this));
            }(std::make_index_sequence<std::size (s_Patterns)>{});
        }
}

void
MemoryManager::Init ()
{
    if (std::exchange (m_initialised, true))
        return;

    InitialiseAllPatterns ();
}
