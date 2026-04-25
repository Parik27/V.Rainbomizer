#include "CPed.hh"
#include "Patterns/Patterns.hh"
#include "memory/GameAddress.hh"
#include "rage.hh"
#include <Utils.hh>
#include <cstdint>
#include <memory>
#include "common/logger.hh"

struct crSkeleton;

class PedRandomizerSkeletonFixes
{
    /*******************************************************/
    template <auto &crSkeleton_SetGlobalMtx>
    static void
    Fix_crSkeleton_SetGlobalMtx (crSkeleton *skelly, uint32_t id,
                                 rage::Mat34V *mat)
    {
        if (id == -1u || !skelly)
            return;

        crSkeleton_SetGlobalMtx (skelly, id, mat);
    }

    /*******************************************************/
    template <auto &crSkeleton_GetGlobalMtx>
    static void
    Fix_crSkeleton_GetGlobalMtx (crSkeleton *skelly, uint32_t id,
                                 rage::Mat34V *mat)
    {
        if (!skelly)
            return;

        if (id == -1u)
            id = 0;

        crSkeleton_GetGlobalMtx (skelly, id, mat);
    }

    /*******************************************************/
    template <auto &CPed_GetBoneIndex>
    static uint32_t
    FixSkeletonBoneCrashes (CPed *ped, uint16_t boneId)
    {
        uint32_t bone = CPed_GetBoneIndex (ped, boneId);
        if (bone == -1u)
            return 0;

        return bone;
    }

    /*******************************************************/
    void
    RegisterTorsoIkProcessFix ()
    {
        uintptr_t p = GameAddress<100022>::Get();

        static uint32_t (*CPed_GetBoneIndex) (CPed *, uint16_t);

        // Folks over at R* didn't check if a ped has a spine before doing
        // operations with their matrices, so this one does it for them.
        for (size_t offset : {0, 16, 40, 63, 86})
            RegisterHook ((void*) (p + offset), CPed_GetBoneIndex,
                          FixSkeletonBoneCrashes<CPed_GetBoneIndex>);
    }

public:
    PedRandomizerSkeletonFixes ()
    {
        // SetGlobalMtx
        REGISTER_JMP_HOOK (
            18,
            100020, Fix_crSkeleton_SetGlobalMtx, void, crSkeleton *, uint32_t,
            rage::Mat34V *);

        // GetGlobalMtx
        REGISTER_JMP_HOOK (17,
                           100021, Fix_crSkeleton_GetGlobalMtx, void, crSkeleton *,
                           uint32_t, rage::Mat34V *);

        RegisterTorsoIkProcessFix ();
    }
} _skellyFixes;
