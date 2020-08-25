#include "sysMemoryAllocator.hh"
#include "Utils.hh"

void (*sysUseAllocator__operator_delete) (void *);
void *(*sysUseAllocator__operator_new) (size_t);

void
sysUseAllocator::operator delete (void *p)
{
    sysUseAllocator__operator_delete (p);
}

void *
sysUseAllocator::operator new (size_t size)
{
    return sysUseAllocator__operator_new (size);
}

void
sysUseAllocator::operator delete[] (void *p)
{
    sysUseAllocator__operator_delete (p);
}

void *
sysUseAllocator::operator new[] (size_t size)
{
    return sysUseAllocator__operator_new (size);
}

void
sysUseAllocator::InitialisePatterns ()
{
    ConvertCall (hook::get_pattern (
                     "? 83 ec 28 ? 8b c1 ? 85 c9 74 ? 8b 15 ? ? ? ? "),
                 sysUseAllocator__operator_delete);

    ReadCall (
        hook::get_pattern ("0f 40 c1 ? 8b c8 e8 ? ? ? ? ? 33 c9 ? 85 c0 74", 6),
        sysUseAllocator__operator_new);
}
