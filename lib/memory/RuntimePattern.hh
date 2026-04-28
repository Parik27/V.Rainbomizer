#pragma once

#include <string_view>
#include <vector>
#include <cstdint>
#include <array>

namespace Rainbomizer {

class RuntimePatternManager
{
    struct RuntimePattern
    {
        struct Resolver
        {
            enum ResolverType
            {
                RESOLVER_OFFSET,
                RESOLVER_REL_REF,
                RESOLVER_BRANCH,
                RESOLVER_RVA
            } type;

            union Data
            {
                int32_t offset;
                int32_t nextInstOffset;
            } data;
        };
        
        std::vector<uint8_t> bytes;
        std::vector<uint8_t> masks;
        std::vector<Resolver> resolvers;

        RuntimePattern () = default;
        RuntimePattern (std::string_view sv);

        bool Check (uint8_t* addr)
        {
            for (size_t i = 0; i < bytes.size (); i++)
                {
                    if ((addr[i] & ~masks[i]) != bytes[i])
                        return false;
                }

            return true;
        }

        uintptr_t Resolve (uintptr_t addr);
    };

    struct RuntimePatternBucketEntry
    {
        int id;
        RuntimePattern pattern;
        int32_t offset;
        int matchIdx = 1;
        uintptr_t resolvedAddress = 0;
    };

    std::array<std::vector<RuntimePatternBucketEntry>, 257> PatternBuckets;

public:

    void ResolvePatterns ();
    void AddPattern (const RuntimePattern& pattern, int id, int matchIdx = 1);

    void ReadFromFile (FILE* file);
    uintptr_t GetResolvedAddress (int id) const;
};

} // namespace Rainbomizer
