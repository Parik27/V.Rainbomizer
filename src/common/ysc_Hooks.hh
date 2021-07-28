#pragma once

#include "Utils.hh"
#include "mission/missions_YscUtils.hh"
#include "events.hh"
#include <cstdint>
#include <scrThread.hh>
#include <map>
#include <vcruntime_string.h>

class YscFunctionHooks
{
    class HookInfo
    {
        uint32_t              Ip                                    = 0;
        bool                  bInitialised                          = false;
        uint32_t              HookedPageIdx                         = 0;
        uint8_t               HookedPage[scrProgram::PAGE_SIZE]     = {0};
        uint8_t               TrampolinePage[scrProgram::PAGE_SIZE] = {0};
        void **               HookedNatives                         = nullptr;
        scrProgram::scrPage **HookedPageList                        = nullptr;

        void
        InstallHook ()
        {
        }

    public:
        void
        Initialise (scrProgram *program)
        {
            if (bInitialised)
                return;

            YscUtils utils (program);

            HookedPageIdx = program->GetTotalPages (Ip) - 1;

            memcpy (HookedPage, program->m_pCodeBlocks[HookedPageIdx],
                    sizeof (HookedPage));

            InstallHook ();
        }

        HookInfo (const HookInfo &other) = delete;
    };

    inline static std::map<uint32_t, HookInfo> m_Hooks;

    static void
    HandleHooks (uint64_t *, uint64_t *, scrProgram *program,
                 scrThreadContext *, bool before)
    {
        if (auto *hook = LookupMap (m_Hooks, program->m_nScriptHash))
            {
                hook->Initialise (program);
            }
    }

public:
    static void
    Initialise ()
    {
        Rainbomizer::Events ().OnRunThread += HandleHooks;
    }
};
