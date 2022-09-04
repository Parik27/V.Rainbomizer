#include "exceptions_Mgr.hh"
#include <common/logger.hh>
#include <common/common.hh>
#include <CTheScripts.hh>
#include <cstring>
#include <algorithm>
#include <exceptions/exceptions_Mgr.hh>

namespace Rainbomizer {
class ExceptionHandler_Threads : public ExceptionHandler
{
    /*******************************************************/
    const char *
    GetStateName (eScriptState state)
    {
        switch (state)
            {
            case eScriptState::KILLED: return "KILLED";
            case eScriptState::RUNNING: return "RUNNING";
            case eScriptState::WAITING: return "WAITING";
            case eScriptState::STATE_3: return "3";
            case eScriptState::STATE_4: return "4";
            }

        return "Unknown";
    }

    /*******************************************************/
    void
    DumpStatics (scrThread *thread)
    {
        FILE *f = Common::GetRainbomizerFile (thread->GetName ()
                                                  + std::string (".stack.bin"),
                                              "w", "logs/threads/");

        fwrite (thread->m_pStack, 8, thread->m_Context.m_nSP, f);
        fclose (f);
    }

    /*******************************************************/
    void
    OnExit () override
    {
        static bool printed = false;

        if (std::exchange (printed, true))
            return;

        if (!CTheScripts::aThreads || !CTheScripts::aThreads->Data)
            return;

        for (auto thread : *CTheScripts::aThreads)
            {
                if (thread->m_Context.m_nThreadId == 0
                    || thread->m_Context.m_nState == eScriptState::KILLED)
                    continue;

                auto program = thread->GetProgram ();

                // Arrow for active thread. Print basic info about the thread
                Logger::LogMessage (
                    "%s %s, Hash: %x, SP: %x, IP: "
                    "%x, FSP: %x, State: %s",
                    thread == scrThread::GetActiveThread () ? "=>" : "  ",
                    thread->GetName (), thread->m_Context.m_nScriptHash,
                    thread->m_Context.m_nSP, thread->m_Context.m_nIp,
                    thread->m_Context.m_nFrameSP,
                    GetStateName (thread->m_Context.m_nState));

                if (!program)
                    continue;

                // Frame Stack (top of the stack frame contains local
                // variables)
                uint64_t *FSP = &thread->m_pStack[thread->m_Context.m_nFrameSP];

                char stack[128] = {0};
                for (int i = 0; i < 8; i++)
                    sprintf (stack + strlen (stack), "%06llx ", *(FSP++));

                Logger::LogMessage ("%s\n", stack);
                DumpStatics (thread);
            }
    }

    /*******************************************************/
    const char *
    GetHandlerName () override
    {
        return "Threads";
    }
};

REGISTER_HANDLER (ExceptionHandler_Threads)

} // namespace Rainbomizer
