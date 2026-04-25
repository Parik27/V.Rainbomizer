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
        REGISTER_HOOK (100048, FixPickupCrash, phCollider *, CEntity*);
        REGISTER_HOOK (100049, FixPickupCrash, phCollider *, CEntity*);
        REGISTER_HOOK (100050, FixPickupCrash, phCollider *, CEntity*);
    }

    ChaosCrashFixes () { RegisterPickupCrash (); }
} g_CrashFixes;
