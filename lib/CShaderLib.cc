#include "Patterns/Patterns.hh"
#include <CShaderLib.hh>
#include <Utils.hh>

sgaShaderGroup *(*rage_sgaShader_ResolveShader) (uint32_t);
void (*rage_sgaShader_destructor) (sgaShaderGroup *);
bool (*rage_sgaShader__LoadShader) (sgaShaderGroup *, char *);

sgaShaderGroup::~sgaShaderGroup () { rage_sgaShader_destructor (this); }

bool
sgaShaderGroup::LoadFile (char *shaderName)
{
    return rage_sgaShader__LoadShader (this, shaderName);
}

sgaShaderGroup *CShaderLib::LookupShader (uint32_t hash)
{
    return rage_sgaShader_ResolveShader (hash);
}

void
CShaderLib::InitialisePatterns ()
{
    ReadCall (hook::get_pattern ("0f b7 05 ? ? ? ? ? 8b d7 ? 8b cb 66 89 ? ? ? "
                                 "? ? e8 ? ? ? ? ? 8b cb 84 c0 74 ?",
                                 20),
              rage_sgaShader__LoadShader);

    ReadCall (
        hook::get_pattern (
            "? 8b cb 84 c0 74 ? e8 ? ? ? ? eb ? e8 ? ? ? ? ? 8b cb e8 ? ? ? ?",
            14),
        rage_sgaShader_destructor);

    ReadCall (hook::get_pattern ("e8 ? ? ? ? 8b c8 e8 ? ? ? ? ? 85 c0 75 ? 81 "
                                 "3d ? ? ? ? 00 02 00 00",
                                 7),
              rage_sgaShader_ResolveShader);
}
