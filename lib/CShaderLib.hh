#pragma once

#include <sysMemoryAllocator.hh>

class sgaShaderGroup : public sysUseAllocator
{
public:
    ~sgaShaderGroup ();

    bool LoadFile (char* shaderName);
};

class CShaderLib
{
public:
    // ? 8b cb 84 c0 74 ? e8 ? ? ? ? eb ? e8 ? ? ? ? ? 8b cb e8 ? ? ? ? (operator delete)
    // e8 ? ? ? ? 8b c8 e8 ? ? ? ? ? 85 c0 75 ? 81 3d ? ? ? ? 00 02 00 00 (second call, Resolve)
    static void InitialisePatterns ();

    static sgaShaderGroup *LookupShader (uint32_t hash);
};
