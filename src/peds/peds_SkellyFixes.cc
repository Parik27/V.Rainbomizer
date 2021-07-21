#include "CPed.hh"
#include "Patterns/Patterns.hh"
#include "rage.hh"
#include <Utils.hh>
#include <cstdint>
#include <memory>
#include "common/logger.hh"

struct crSkeleton;

/*******************************************************/
/* These are here because of a bug in clang            */
/*******************************************************/
template <auto &crSkeleton_PartialUpdate>
static void
Fix_crSkeleton_PartialUpdate (crSkeleton *skelly, uint32_t id, bool fullUpdate)
{
    if (id == -1)
        id = 0;

    crSkeleton_PartialUpdate (skelly, id, fullUpdate);
}
/*******************************************************/
template <size_t i>
void
RegisterPartialUpdateHook ()
{
    using f_crSkeleton_PartialUpdate = void (*) (crSkeleton *, uint32_t, bool);

    static hook::pattern p (
        "? 8b c4 ? 89 ? ? ? 89 ? ? ? 89 ? ? 55 41 56 41 57 ? "
        "8d ? a1 ? 81 ec d0 00 00 00 ? 8b 31");

    if (p.size () <= i)
        return;

    static f_crSkeleton_PartialUpdate s_PartialUpdateFunc;

    RegisterJmpHook<15> (p.get (i).get<void> (), s_PartialUpdateFunc,
                         Fix_crSkeleton_PartialUpdate<s_PartialUpdateFunc>);
}

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
        hook::pattern p (
            "e8 ? ? ? ? ? 8b 4f 30 ba 01 5c 00 00 8b d8 e8 ? ? ? ? ? 8b 4f 30 "
            "44 8b f0 ba f0 60 00 00 44 89 ? ? ? ? ? e8 ? ? ? ? ? 8b 4f 30 ba "
            "f1 60 00 00 44 8b e0 89 ? ? ? ? ? e8 ? ? ? ? ? 8b 4f 30 ba f2 60 "
            "00 00 44 8b e8 89 ? ? ? ? ? e8");

        static uint32_t (*CPed_GetBoneIndex) (CPed *, uint16_t);

        // Folks over at R* didn't check if a ped has a spine before doing
        // operations with their matrices, so this one does it for them.
        for (size_t offset : {0, 16, 40, 63, 86})
            RegisterHook (p.get_first (offset), CPed_GetBoneIndex,
                          FixSkeletonBoneCrashes<CPed_GetBoneIndex>);
    }

public:
    PedRandomizerSkeletonFixes ()
    {
        // SetGlobalMtx
        REGISTER_JMP_HOOK (
            18,
            "40 55 ? 8d ? ? a9 ? 81 ec d0 00 00 00 ? 8b 41 08 ? 8b c9 ? 85 c0",
            0, Fix_crSkeleton_SetGlobalMtx, void, crSkeleton *, uint32_t,
            rage::Mat34V *);

// PartialUpdate
#ifdef PARTIAL_UPDATE_CRASH_FIX
        RegisterPartialUpdateHook<0> ();
        RegisterPartialUpdateHook<1> ();
#endif

        // GetGlobalMtx
        REGISTER_JMP_HOOK (17,
                           "40 55 ? 8d ? ? a9 ? 81 ec d0 00 00 00 ? 8b d0 ? 8b "
                           "41 08 8b c2 ? c1 e0 06 ? 03 41 18 ",
                           0, Fix_crSkeleton_GetGlobalMtx, void, crSkeleton *,
                           uint32_t, rage::Mat34V *);

        RegisterTorsoIkProcessFix ();
    }
} _skellyFixes;
