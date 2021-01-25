#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <functional>

#include <Utils.hh>
#include <scrThread.hh>

/* Class for working with scrProgram. */
class YscUtils
{
    scrProgram *m_pProgram;

public:
    YscUtils (scrProgram *program) : m_pProgram (program){};

    void FindCodePattern (const std::string &                       pattern,
                          std::function<void (hook::pattern_match)> CB);

    void FindString (const char *str, void (*CB) (char *));

    void ExtendCode (uint32_t newSize);

    inline scrProgram::scrPage *
    AddNewPage ()
    {
        uint32_t totalPages
            = m_pProgram->GetTotalPages (m_pProgram->m_nCodeSize);
        ExtendCode (totalPages * scrProgram::PAGE_SIZE);

        return &m_pProgram->m_pCodeBlocks[totalPages];
    }

    uint32_t GetCodeOffset (uint8_t *code);

    inline scrProgram *
    GetProgram ()
    {
        return m_pProgram;
    }

    /* Make sure the script doesn't call WAIT or a native that changes the
     * script's state. The script will stop execution there and not continue! */
    template <typename... Args>
    void
    CallScriptFunction (uint32_t ip, Args... args)
    {
        const int STACK_SIZE = 512;

        // Create dummy stack for creating a context for the function call.
        scrThread  thread;
        scrThread *prevActiveThread
            = std::exchange (scrThread::GetActiveThread (), &thread);
        uint32_t prevActiveThreadIp = prevActiveThread->m_Context.m_nIp;
        
        auto stack = std::make_unique<uint64_t[]> (STACK_SIZE);
        memset (stack.get(), 0, STACK_SIZE * 8);
        thread.m_pStack        = stack.get ();
        thread.m_Context.m_nIp = ip;
        thread.m_Context.m_nStackSize    = STACK_SIZE;
        thread.m_Context.m_nState = eScriptState::RUNNING;

        // Push arguments and return address to the thread stack.
        (..., thread.Push64 (args));
        thread.Push64 (0); // Returning to 0 terminates the script.

        thread.Run (stack.get (), scrThread::sm_pGlobals, m_pProgram,
                    &thread.m_Context);

        scrThread::GetActiveThread () = prevActiveThread;
        prevActiveThread->m_Context.m_nIp = prevActiveThreadIp;
    }
};
