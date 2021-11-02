#include "base.hh"
#include <Utils.hh>
#include <CTheScripts.hh>

#include <common/events.hh>

using namespace NativeLiterals;

DebugInterface::DebugInterface ()
{
    DebugInterfaceManager::RegisterInterface (*this);
}
DebugInterfaceManager::DebugInterfaceManager ()
{
    InitialiseAllComponents ();
    Rainbomizer::Events ().OnRunThread += RunThreadHook;
}

void
DebugInterfaceManager::RunThreadHook (uint64_t *stack, uint64_t *globals,
                                      scrProgram *      program,
                                      scrThreadContext *ctx)
{
    for (auto i : GetDebugInterfaces ())
        {
            i->Process (stack, globals, program, ctx);
        }

    if (m_BlockControls && !m_EnableFloating)
        "DISABLE_ALL_CONTROL_ACTIONS"_n(0);
}

DebugInterfaceManager g_DebugInterfaceManager;
