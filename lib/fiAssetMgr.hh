#pragma once

#include "memory/GameAddress.hh"
class fiAssetMgr
{
public:
    inline static GameVariable<fiAssetMgr, 100146> sm_Instance{};

    void
    PushFolder (const char *folder)
    {
        return GameFunction<100148, void (fiAssetMgr *, const char *)>::Call (
            this, folder);
    }

    void
    PopFolder ()
    {
        return GameFunction<100147, void (fiAssetMgr *)>::Call (this);
    }
};
