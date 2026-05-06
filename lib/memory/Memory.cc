#include "Memory.hh"
#include "GameVersion.hh"
#include "common/common.hh"
#include "memory/RuntimePattern.hh"
#include <utility>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Patterns/Patterns.hh>

#define GAME_INTERNAL_NAME "GTA3"

void ResolveGameAddresses (const Rainbomizer::RuntimePatternManager &manager);

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
}

void
MemoryManager::Init ()
{
    if (std::exchange (m_initialised, true))
        return;

    InitialiseAllPatterns ();
}
