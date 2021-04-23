#include <Utils.hh>
#include "Patterns/Patterns.hh"
#include "phBound.hh"

void (*phBoundComposite__AllocateTypeAndIncludeFlags) (phBoundComposite *) = nullptr;

void
phBound::InitialisePatterns ()
{
    ConvertCall (hook::get_pattern (
                     "40 53 ? 83 ec 20 0f b7 81 a0 00 00 00 ? 8b d9 "),
                 phBoundComposite__AllocateTypeAndIncludeFlags);
}

void
phBoundComposite::AllocateTypeAndIncludeFlags ()
{
    return phBoundComposite__AllocateTypeAndIncludeFlags (this);
}
