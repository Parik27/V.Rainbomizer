#pragma once

#include "Patterns/Patterns.hh"
#include <cstdint>
#include <string_view>

struct Pattern
{
    using resolver_type = uintptr_t (*) (uintptr_t);

    uintptr_t        address;
    std::string_view pattern_str;
    resolver_type    resolver = Offset<0> ();
    uint32_t         matchIdx = 0;

    template<int32_t offset>
    consteval static resolver_type Offset ()
    {
        return [] (uintptr_t ptr) {
            return uintptr_t(ptr + offset);
        };
    }

    template<int32_t dataOffset, int32_t nextInstOffset>
    consteval static resolver_type RelRef ()
    {
        return [] (uintptr_t ptr) {
            int32_t  offset = *(int32_t *) (ptr + dataOffset);
            return (ptr + offset + nextInstOffset);
        };
    }

    template<int32_t dataOffset>
    consteval static resolver_type RvaRef ()
    {
        return [] (uintptr_t ptr) {
            uint32_t offset = *(uint32_t*) (ptr + dataOffset);
            return (hook::getRVA (offset));
        };
    }

    template<int32_t dataOffset>
    consteval static resolver_type Branch ()
    {
        return [] (uintptr_t ptr) {
            return uintptr_t(injector::GetBranchDestination (ptr + dataOffset));
        };
    }

    template<auto ... Args>
    consteval static resolver_type MakeResolver ()
    {
        return [] (uintptr_t ptr) {
            return (..., (ptr = Args(ptr)));
        };
    }
};

#if defined(_MSC_VER) && !defined(__clang__)
    #pragma section(".pat_ids$a", read)
    #pragma section(".pat_ids$m", read)
    #pragma section(".pat_ids$z", read)
    // Markers to find the start and end of the integer array
    __declspec(allocate(".pat_ids$a")) inline const uint32_t pat_ids_start = 0;
    __declspec(allocate(".pat_ids$z")) inline const uint32_t pat_ids_end = 0;

    #define EMBED_ID(id) \
        __declspec(allocate(".pat_ids$m")) \
        inline static const uintptr_t entry = id
#else
    extern "C" const uintptr_t __start_pat_ids[];
    extern "C" const uintptr_t __stop_pat_ids[];

    #define EMBED_ID(id) \
        __attribute__((used, section(".pat_ids"), aligned(4))) \
        inline static const uintptr_t entry = id
#endif

template<uintptr_t ID>
struct PatternTracker {
    EMBED_ID(ID);
};
