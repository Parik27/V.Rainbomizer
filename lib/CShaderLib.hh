#pragma once

#include "memory/GameAddress.hh"
#include <sysMemoryAllocator.hh>

class sgaShaderGroup : public sysUseAllocator
{
public:
    ~sgaShaderGroup ()
    {
        GameFunction<100145, void(sgaShaderGroup*)>::Call (this);
    }

    bool
    LoadFile (char *shaderName)
    {
        return GameFunction<100144, bool (sgaShaderGroup *, char *)>::Call (
            this, shaderName);
    }
};

class CShaderLib
{
public:
    inline static GameFunction<100143, sgaShaderGroup*(uint32_t)> LookupShader{};
};
