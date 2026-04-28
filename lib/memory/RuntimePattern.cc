#include "RuntimePattern.hh"
#include "common/logger.hh"
#include "injector/injector.hpp"
#include <cstdint>
#include <memory/MemorySignature.hh>

#include <cstdint>
#include <windows.h>
#include <psapi.h>
#include <ranges>
#include <string>
#include <algorithm>

using namespace Rainbomizer;

static uint8_t *
GetGameAddressStart ()
{
    HMODULE hModule = GetModuleHandleW (nullptr);
    return reinterpret_cast<uint8_t *> (hModule);
}

static uint8_t *
GetGameAddressEnd ()
{
    HMODULE hModule = GetModuleHandleW (nullptr);
    if (!hModule)
        return nullptr;

    MODULEINFO moduleInfo{};
    if (!GetModuleInformation (GetCurrentProcess (), hModule, &moduleInfo,
                               sizeof (moduleInfo)))
        return nullptr;

    return reinterpret_cast<uint8_t *> (
        reinterpret_cast<std::uintptr_t> (moduleInfo.lpBaseOfDll)
        + moduleInfo.SizeOfImage);
}

uintptr_t
RuntimePatternManager::RuntimePattern::Resolve (uintptr_t addr)
{
    for (auto &resolver : resolvers)
        {
            switch (resolver.type)
                {
                case Resolver::RESOLVER_OFFSET:
                    addr = addr + resolver.data.offset;
                    break;

                case Resolver::RESOLVER_BRANCH:
                    addr = injector::GetBranchDestination (addr).as_int ();
                    break;

                    case Resolver::RESOLVER_REL_REF: {
                        uint32_t offset = injector::ReadMemory<uint32_t> (addr);
                        addr = addr + offset + resolver.data.nextInstOffset;
                        break;
                    }

                    case Resolver::RESOLVER_RVA: {
                        uint32_t offset = injector::ReadMemory<uint32_t> (addr);

                        addr = addr + offset
                               + uintptr_t (GetGameAddressStart ());
                    }
                }
        }

    return addr;
}

void
RuntimePatternManager::ResolvePatterns ()
{
    uint8_t *start = GetGameAddressStart ();
    uint8_t *end   = GetGameAddressEnd ();

    while (start != end)
        {
            for (auto &pattern : PatternBuckets[*start])
                {
                    if (pattern.resolvedAddress == 0
                        && pattern.pattern.Check (start + pattern.offset))
                        {
                            pattern.matchIdx -= 1;
                            if (pattern.matchIdx == 0)
                                {
                                    pattern.resolvedAddress
                                        = pattern.pattern.Resolve (
                                            uintptr_t (start) + pattern.offset);

                                    Rainbomizer::Logger::LogMessage (
                                        "%d resolved to %x", pattern.id,
                                        pattern.resolvedAddress);
                                }
                        }
                }

            for (auto &pattern : PatternBuckets.back ())
                {
                    if (pattern.resolvedAddress == 0
                        && pattern.pattern.Check (start + pattern.offset))
                        {
                            pattern.matchIdx -= 1;
                            if (pattern.matchIdx == 0)
                                {
                                    pattern.resolvedAddress
                                        = pattern.pattern.Resolve (
                                            uintptr_t (start) + pattern.offset);

                                    Rainbomizer::Logger::LogMessage (
                                        "%d resolved to %x", pattern.id,
                                        pattern.resolvedAddress);
                                }
                        }
                }

            start++;
        }
}

void
RuntimePatternManager::AddPattern (const RuntimePattern &pattern, int id, int matchIdx)
{
    for (int32_t i = 0; i < pattern.bytes.size (); i++)
        {
            if (pattern.masks[i] == 0)
                {
                    PatternBuckets[pattern.bytes[i]].push_back (
                        RuntimePatternBucketEntry{id, pattern, -i, matchIdx});

                    return;
                }
        }

    PatternBuckets.back ().push_back (
        RuntimePatternBucketEntry{id, pattern, 0, matchIdx});
}

static inline std::string
TrimString (const std::string &s)
{
    size_t begin = 0;
    while (begin < s.size ()
           && std::isspace (static_cast<unsigned char> (s[begin])))
        begin++;

    size_t end = s.size ();
    while (end > begin
           && std::isspace (static_cast<unsigned char> (s[end - 1])))
        end--;

    return s.substr (begin, end - begin);
}

static inline std::vector<std::string>
SplitCommaSeparated (const std::string &line)
{
    std::vector<std::string> parts;
    size_t                   start = 0;

    while (start <= line.size ())
        {
            size_t pos = line.find (',', start);
            if (pos == std::string::npos)
                {
                    parts.push_back (TrimString (line.substr (start)));
                    break;
                }

            parts.push_back (TrimString (line.substr (start, pos - start)));
            start = pos + 1;
        }

    return parts;
}

static inline bool
ParseInteger (const std::string &text, int32_t &out)
{
    if (text.empty ())
        return false;

    char *end   = nullptr;
    long  value = std::strtol (text.c_str (), &end, 0);
    if (end == text.c_str () || *end != '\0')
        return false;

    out = static_cast<int32_t> (value);
    return true;
}

void
RuntimePatternManager::ReadFromFile (FILE *file)
{
    if (file == nullptr)
        return;

    char buffer[4096];

    std::string                           currentId;
    std::string                           currentPattern;
    std::vector<RuntimePattern::Resolver> currentResolvers;
    int32_t                               currentOffset = 0;
    bool                                  haveOffset    = false;
    bool                                  havePattern   = false;

    auto flushCurrent = [&] () {
        if (!havePattern)
            {
                currentId.clear ();
                currentPattern.clear ();
                currentResolvers.clear ();
                haveOffset = false;
                return;
            }

        RuntimePattern pattern (currentPattern);

        pattern.resolvers = currentResolvers;

        int id;
        ParseInteger (currentId, id);
        AddPattern (pattern, id);

        currentId.clear ();
        currentPattern.clear ();
        currentResolvers.clear ();
        haveOffset  = false;
        havePattern = false;
    };

    while (std::fgets (buffer, sizeof (buffer), file) != nullptr)
        {
            std::string line = TrimString (buffer);
            if (line.empty () || line[0] == '#' || line.rfind ("//", 0) == 0)
                continue;

            auto parts = SplitCommaSeparated (line);
            if (parts.empty ())
                continue;

            std::string key = parts[0];
            std::transform (key.begin (), key.end (), key.begin (),
                            [] (unsigned char c) {
                                return static_cast<char> (std::tolower (c));
                            });

            if (key == "id")
                {
                    flushCurrent ();
                    currentId = (parts.size () > 1) ? parts[1] : std::string ();
                    continue;
                }

            if (key == "pattern")
                {
                    currentPattern
                        = (parts.size () > 1) ? parts[1] : std::string ();
                    havePattern = true;
                    continue;
                }

            if (key == "rel_ref" || key == "rva_ref" || key == "branch"
                || key == "offset")
                {
                    RuntimePattern::Resolver resolver{};
                    resolver.type
                        = (key == "offset")
                              ? RuntimePattern::Resolver::RESOLVER_OFFSET
                          : (key == "rel_ref")
                              ? RuntimePattern::Resolver::RESOLVER_REL_REF
                          : (key == "branch")
                              ? RuntimePattern::Resolver::RESOLVER_BRANCH
                              : RuntimePattern::Resolver::RESOLVER_RVA;

                    int32_t value = 0;
                    if (parts.size () > 1 && ParseInteger (parts[1], value))
                        resolver.data.offset = value;
                    else
                        resolver.data.offset = 0;

                    currentResolvers.push_back (resolver);
                    continue;
                }
        }

    flushCurrent ();
}

uintptr_t RuntimePatternManager::GetResolvedAddress (int id) const
{
    for (auto &bucket : PatternBuckets)
        {
            for (auto &entry : bucket)
                {
                    if (entry.id == id)
                        {
                            return entry.resolvedAddress;
                        }
                }
        }

    return 0;
}

RuntimePatternManager::RuntimePattern::RuntimePattern (std::string_view sv)
{
    for (const auto i :
         std::views::split (sv, ' ')
             | std::views::filter ([] (const auto i) { return i.size () > 0; }))
        {
            auto [byte, mask] = MemorySignature::NibbleMaskFromString (i);
            bytes.push_back(byte);
            masks.push_back(mask);
        }
}
