#include "exceptions_Mgr.hh"

#include <windows.h>
#include <dbghelp.h>
#include <optional>
#include <sstream>
#include "injector/injector.hpp"

#include "common/logger.hh"

const int MAX_SYMBOL_SIZE = 256;

namespace Rainbomizer {
class ExceptionHandler_BackTrace : public ExceptionHandler
{
    STACKFRAME64 m_StackFrame;
    CONTEXT m_Context;
    bool m_bHasSymHandler;
    DWORD m_OriginalSymOptions;
    uint8_t m_SymBuffer[sizeof (SYMBOL_INFO) + MAX_SYMBOL_SIZE];

    /*******************************************************/
    void
    InitialiseStackFrame (CONTEXT *ctx)
    {
        memset (&m_StackFrame, 0, sizeof (m_StackFrame));
        memset (&m_SymBuffer, 0, sizeof (m_SymBuffer));
        memcpy (&m_Context, ctx, sizeof (CONTEXT));
        
        m_StackFrame.AddrPC.Offset    = ctx->Rip;
        m_StackFrame.AddrStack.Offset = ctx->Rsp;
        m_StackFrame.AddrFrame.Offset = ctx->Rbp;
        m_StackFrame.AddrPC.Mode      = AddrModeFlat;
        m_StackFrame.AddrStack.Mode   = AddrModeFlat;
        m_StackFrame.AddrFrame.Mode   = AddrModeFlat;
    }

    /*******************************************************/
    void
    InitialiseSymbolHandler ()
    {
        if (SymInitialize(GetCurrentProcess(), 0, TRUE))
            {
                m_OriginalSymOptions = SymSetOptions (
                    SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES
                    | SYMOPT_NO_PROMPTS | SYMOPT_FAIL_CRITICAL_ERRORS);
                
                m_bHasSymHandler = true;
                return;
            }
        m_bHasSymHandler = false;
    }

    /*******************************************************/
    void
    DestroySymbolHandler ()
    {
        if (m_bHasSymHandler)
            {
                SymSetOptions (m_OriginalSymOptions);
                SymCleanup(GetCurrentProcess());
            }
    }
    
    /*******************************************************/
    bool
    WalkStack (CONTEXT* ctx)
    {
        return ::StackWalk64 (IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess (),
                              GetCurrentThread (), &m_StackFrame, &m_Context,
                              NULL, NULL, NULL, NULL);
    }

    /*******************************************************/
    auto
    GetSymbol (uint64_t PC)
    {
        if (m_bHasSymHandler)
            {
                SYMBOL_INFO &symbol
                    = *reinterpret_cast<SYMBOL_INFO *> (m_SymBuffer);

                symbol.SizeOfStruct = sizeof (SYMBOL_INFO);
                symbol.MaxNameLen   = MAX_SYMBOL_SIZE;

                uint64_t displacement;

                if (SymFromAddr (GetCurrentProcess (), PC, &displacement,
                                 &symbol))
                    return std::make_pair (displacement,
                                           (const char *) symbol.Name);
            }

        return std::make_pair (0ULL, "??");
    }

    /*******************************************************/
    HMODULE
    GetModuleFromAddress (uint64_t Address)
    {
        HMODULE module;
        if (GetModuleHandleExA (
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                    | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (char *) Address, &module))
            return module;

        return nullptr;
    }

    /*******************************************************/
    std::string
    FindModuleName (HMODULE module)
    {
        char moduleName[256] = {0};
        if (!module
            || !GetModuleFileNameA (module, moduleName, sizeof (moduleName)))
            return "unknown";

        std::string modName = moduleName;

        // Remove directory names and stuff
        int fileStart = modName.rfind ('\\');
        fileStart
            = (fileStart == modName.npos) ? modName.rfind ('/') : fileStart;

        return modName.substr (fileStart + 1);
    }

    /*******************************************************/
    auto
    GetModule (uint64_t PC)
    {
        HMODULE module = GetModuleFromAddress (PC);
        return std::make_pair (FindModuleName (module),
                               PC - uintptr_t (module));
    }
    
    /*******************************************************/
    MEMORY_BASIC_INFORMATION
    VirtualQueryWrapper (uintptr_t lpAddress)
    {
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery ((void *) lpAddress, &mbi, sizeof (mbi));

        return mbi;
    }
    
    /*******************************************************/
    bool
    CheckIsPointerValid (uintptr_t ptr)
    {
        auto mbi = VirtualQueryWrapper(ptr);
        return (mbi.State & MEM_COMMIT);
    }
    
    /*******************************************************/
    std::string
    GetPatternForAddress (uintptr_t Addr)
    {
        if (!CheckIsPointerValid(Addr - 10) ||
            !CheckIsPointerValid(Addr + 10))
                return "?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ??";

        char buffer[64] = {0};

        for (uintptr_t i = Addr - 10; i <= Addr + 10; i++)
            sprintf (buffer + strlen (buffer), "%02X ",
                     injector::ReadMemory<uint8_t> (i, true));

        return buffer;
    }

    /*******************************************************/
    std::string
    GetAddressDescription (uintptr_t Addr)
    {
        char buf[512]             = {0};
        auto [symOffset, symName] = GetSymbol (Addr);
        auto [modName, modOffset] = GetModule (Addr);

        sprintf (buf, "0x%016llx: %s+0x%llx in %s (+0x%llx)", Addr, symName,
                 symOffset, modName.c_str (), modOffset);

        return buf;
    }

    /*******************************************************/
    void
    PrintUnhandledException (_EXCEPTION_POINTERS *ep)
    {
        uintptr_t addr = (uintptr_t) ep->ExceptionRecord->ExceptionAddress;
        auto      dwExceptionCode = ep->ExceptionRecord->ExceptionCode;
        
        auto [modName, modOffset] = GetModule (addr);

        Logger::LogMessage ("Unhandled exception at 0x%016llx in %s (+0x%llx)",
                            addr, modName.c_str (), modOffset);
        
        if (dwExceptionCode == EXCEPTION_IN_PAGE_ERROR || dwExceptionCode == EXCEPTION_ACCESS_VIOLATION)
            {
                // From Ultimate ASI Loader
                auto rw = (DWORD) ep->ExceptionRecord
                              ->ExceptionInformation[0]; // read or write?
                auto addr = (ULONG_PTR) ep->ExceptionRecord
                                ->ExceptionInformation[1]; // which address?

                Logger::LogMessage ("%s 0x%016llx",
                                    rw == 0 ? "reading location"
                                            : rw == 1 ? "writing location"
                                                      : rw == 8 ? "dep at" : "",
                                    addr);
            }
    }
    
    /*******************************************************/
    void
    OnException (_EXCEPTION_POINTERS *ep) override
    {
        Logger::LogMessage ("%s", GetHandlerName ());

        PrintUnhandledException (ep);
        InitialiseStackFrame (ep->ContextRecord);
        InitialiseSymbolHandler ();

        const uint8_t STACK_WALK_LIMIT = 64;
        for (uint8_t i = 0;
             i < STACK_WALK_LIMIT && WalkStack (ep->ContextRecord); i++)
            {
                Logger::LogMessage (
                    "%s %s (SP: %llx)", (i == 0) ? "=> " : "   ",
                    GetAddressDescription (m_StackFrame.AddrPC.Offset).c_str (),
                    m_StackFrame.AddrFrame.Offset);
                Logger::LogMessage (
                    "    %s",
                    GetPatternForAddress (m_StackFrame.AddrPC.Offset).c_str ());
            }

        DestroySymbolHandler ();
    }

    /*******************************************************/
    const char *
    GetHandlerName () override
    {
        return "Backtrace";
    }
};

REGISTER_HANDLER (ExceptionHandler_BackTrace);
}; // namespace Rainbomizer
