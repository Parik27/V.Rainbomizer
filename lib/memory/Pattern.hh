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
    uint32_t         matchIdx;

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
