#pragma once

#include <cstdint>

class sysUseAllocator
{
public:
    void operator delete (void *p);
    void operator delete[] (void *p);

    void *operator new (size_t size);
    void *operator new[] (size_t size);

    static void InitialisePatterns ();
};
