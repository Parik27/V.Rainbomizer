#include "exceptions_Mgr.hh"
#include "common/logger.hh"

#include <windows.h>

namespace Rainbomizer {

/* Logs registers */
class ExceptionHandler_Registers : public ExceptionHandler
{
    /*******************************************************/
    void
    OnException (_EXCEPTION_POINTERS *ep) override
    {
        /* x64 registers, by the way. */
        Logger::LogMessage ("General Registers:\n");

        auto PrintRegister = [ep] (const char *name, auto value) {
            Logger::LogMessage ("%s: 0x%016llx (%lld)", name, value);
        };

        PrintRegister ("RAX", ep->ContextRecord->Rax);
        PrintRegister ("RBX", ep->ContextRecord->Rbx);
        PrintRegister ("RCX", ep->ContextRecord->Rcx);
        PrintRegister ("RDX", ep->ContextRecord->Rdx);
        PrintRegister ("RSI", ep->ContextRecord->Rsi);
        PrintRegister ("RDI", ep->ContextRecord->Rdi);
        PrintRegister ("RBP", ep->ContextRecord->Rbp);
        PrintRegister ("RSP", ep->ContextRecord->Rsp);
        PrintRegister ("RIP", ep->ContextRecord->Rip);

        PrintRegister ("R8", ep->ContextRecord->R8);
        PrintRegister ("R9", ep->ContextRecord->R9);
        PrintRegister ("R10", ep->ContextRecord->R10);
        PrintRegister ("R11", ep->ContextRecord->R11);
        PrintRegister ("R12", ep->ContextRecord->R12);
        PrintRegister ("R13", ep->ContextRecord->R13);
        PrintRegister ("R14", ep->ContextRecord->R14);
        PrintRegister ("R15", ep->ContextRecord->R15);

        Logger::LogMessage ("Segment Registers:");

        Logger::LogMessage ("CS: %04x", ep->ContextRecord->SegCs);
        Logger::LogMessage ("DS: %04x", ep->ContextRecord->SegDs);
        Logger::LogMessage ("ES: %04x", ep->ContextRecord->SegEs);
        Logger::LogMessage ("FS: %04x", ep->ContextRecord->SegFs);
        Logger::LogMessage ("GS: %04x", ep->ContextRecord->SegGs);
        Logger::LogMessage ("SS: %04x", ep->ContextRecord->SegSs);
    }

    /*******************************************************/
    const char *
    GetHandlerName () override
    {
        return "Registers";
    }
};

REGISTER_HANDLER (ExceptionHandler_Registers)

} // namespace Rainbomizer
