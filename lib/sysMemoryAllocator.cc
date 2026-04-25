#include "sysMemoryAllocator.hh"
#include "memory/GameAddress.hh"

static GameFunction<100135, void*(void*)> sysUseAllocator__operator_delete{};
static GameFunction<100136, void*(size_t)> sysUseAllocator__operator_new{};

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
