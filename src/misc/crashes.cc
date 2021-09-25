#include <cstdio>
#include <CEntity.hh>
#include <phCollider.hh>
#include <Utils.hh>

class ChaosCrashFixes
{
    template <auto &CEntity__GetColliderNonConst>
    static phCollider *
    FixPickupCrash (CEntity *entity)
    {
        static phCollider s_Default;
        s_Default.nType = 0;

        phCollider *collider = CEntity__GetColliderNonConst (entity);
        return collider ? collider : &s_Default;
    }

public:
    void
    RegisterPickupCrash ()
    {
        auto pattern
            = hook::pattern ("e8 ? ? ? ? ? ? ? ? ? 8d ? ? ? 8b c8 83 78 08 00")
                  .count (3);

        using FuncType = phCollider *(*) (CEntity *);
        static FuncType CEntity__GetColliderNonConst21 = nullptr;

        pattern.for_each_result ([] (hook::pattern_match m) {
            RegisterHook (m.get<void> (), CEntity__GetColliderNonConst21,
                          FixPickupCrash<CEntity__GetColliderNonConst21>);
        });
    }

    ChaosCrashFixes () { RegisterPickupCrash (); }
} g_CrashFixes;
