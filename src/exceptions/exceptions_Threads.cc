#include "exceptions_Mgr.hh"
#include "common/logger.hh"
#include <CTheScripts.hh>
#include <cstring>

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
    OnExit () override
    {
        static bool printed = false;

        if (std::exchange(printed, true))
            return;
        
        if (!CTheScripts::aThreads || !CTheScripts::aThreads->Data)
            return;

        for (int i = 0; i < CTheScripts::aThreads->Size; i++)
            {
                auto thread = CTheScripts::aThreads->Data[i];
                if (thread->m_Context.m_nThreadId == 0
                    || thread->m_Context.m_nState == eScriptState::KILLED)
                    continue;

                auto program = thread->GetProgram ();

                // Arrow for active thread. Print basic info about the thread
                Logger::LogMessage (
                    "%s %s, Hash: %x, SP: %x, IP: "
                    "%x, FSP: %x, State: %s",
                    thread == scrThread::GetActiveThread () ? "=>" : "  ",
                    thread->m_szScriptName, thread->m_Context.m_nScriptHash,
                    thread->m_Context.m_nSP, thread->m_Context.m_nIp,
                    thread->m_Context.m_nFrameSP,
                    GetStateName (thread->m_Context.m_nState));

                // Disassemble bytes around it:
                if (!program)
                    continue;

                uint32_t instructionsPrinted = 0;
                for (uint32_t offset = 0; offset < program->m_nCodeSize;
                     offset += scrThread::FindInstSize (program, offset))
                    {
                        if (program->m_nScriptHash == "player_timetable_scene"_joaat)
                            Rainbomizer::Logger::LogMessage("%d", offset);

                        if (program->m_nScriptHash
                                == "player_timetable_scene"_joaat
                            || offset + 10 >= thread->m_Context.m_nIp)
                            {
                                char instruction[1024] = {0};
                                scrThread::DisassemblInsn (instruction, program,
                                                           offset);

                                // Arrow for the current instruction
                                Logger::LogMessage (
                                    "%s %s",
                                    offset == thread->m_Context.m_nIp ? "=>"
                                                                      : "  ",
                                    instruction);
                                
                                instructionsPrinted++;
                            }

                        if (instructionsPrinted > 5
                            && program->m_nScriptHash
                                   != "player_timetable_scene"_joaat)
                            break;
                    }

                // Frame Stack (top of the stack frame contains local variables)
                uint32_t *FSP = &thread->m_pStack[thread->m_Context.m_nFrameSP];

                char stack[128] = {0};
                for (int i = 0; i < 8; i++)
                    sprintf (stack + strlen (stack), "%06x", *(FSP++));

                Logger::LogMessage("%s\n", stack);
            }
    }

    /*******************************************************/
    const char *
    GetHandlerName () override
    {
        return "Threads";
    }
};

    //REGISTER_HANDLER (ExceptionHandler_Threads)

}; // namespace Rainbomizer
