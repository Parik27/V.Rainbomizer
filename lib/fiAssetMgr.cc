#include "fiAssetMgr.hh"
#include "Patterns/Patterns.hh"
#include "Utils.hh"

// e8 ? ? ? ? ? 8b 0d ? ? ? ? ? 8b ? ff 50 08 83 3d ? ? ? ? 01

// ? 8d 0d ? ? ? ? e8 ? ? ? ? ? 8b 05 ? ? ? ? ? 8d 15 ? ? ? ? ? 8b 48 08

void (*rage__fiAssetManager__PushFolder) (fiAssetMgr *, const char *);
void (*rage__fiAssetManager__PopFolder) (fiAssetMgr *);

void
fiAssetMgr::InitialisePatterns ()
{
    ReadCall (
        hook::get_pattern (
            "e8 ? ? ? ? ? 8b 0d ? ? ? ? ? 8b ? ff 50 08 83 3d ? ? ? ? 01 "),
        rage__fiAssetManager__PushFolder);

    ReadCall (hook::get_pattern ("? 8d 0d ? ? ? ? e8 ? ? ? ? ? 8b 05 ? ? ? ? "
                                 "? 8d 15 ? ? ? ? ? 8b 48 08 ",
                                 7),
              rage__fiAssetManager__PopFolder);

    sm_Instance = GetRelativeReference<fiAssetMgr> (
        "? 8d 0d ? ? ? ? e8 ? ? ? ? ? 8b 05 ? ? ? ? "
        "? 8d 15 ? ? ? ? ? 8b 48 08 ",
        3, 7);
}

void
fiAssetMgr::PushFolder (const char *folder)
{
    return rage__fiAssetManager__PushFolder (this, folder);
}

void
fiAssetMgr::PopFolder ()
{
    return rage__fiAssetManager__PopFolder (this);
}
