#include "exceptions_Mgr.hh"
#include "common/logger.hh"
#include <windows.h>
#include <tuple>

namespace Rainbomizer
{

/* Prints stack dump at exception                      */
/*******************************************************/
class ExceptionHandler_Stack : public ExceptionHandler
{

    uint8_t m_nCurrentCol = 0;
    char m_sLineBuffer[1024] = {0};
    
    /*******************************************************/
    MEMORY_BASIC_INFORMATION
    VirtualQueryWrapper (uintptr_t lpAddress)
    {
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery ((void *) lpAddress, &mbi, sizeof (mbi));

        return mbi;
    }

    /* Returns base, top, bottom of the stack given the top */
    /*******************************************************/
    auto
    GetStackBounds (uintptr_t top)
    {
        uintptr_t base, bottom;

        // Base
        auto mbi = VirtualQueryWrapper (top);
        base     = uintptr_t (mbi.AllocationBase);

        // uncommited region
        mbi = VirtualQueryWrapper (base);

        // guard page
        mbi = VirtualQueryWrapper (uintptr_t (mbi.BaseAddress)
                                   + mbi.RegionSize);

        // committed region
        mbi = VirtualQueryWrapper (uintptr_t (mbi.BaseAddress)
                                   + mbi.RegionSize);

        bottom = uintptr_t (mbi.BaseAddress) + mbi.RegionSize;
        return std::make_tuple(base, top, bottom);
    }

    /*******************************************************/
    void
    FlushBuffer ()
    {
        Logger::LogMessage (m_sLineBuffer);
        memset (m_sLineBuffer, 0, sizeof (m_sLineBuffer));

        static_assert (sizeof (m_sLineBuffer) == 1024);
    }

    /*******************************************************/
    bool
    PrintStackWord (uintptr_t top, uint32_t& offset, uintptr_t bottom)
    {
        const uint8_t TOTAL_COLS = 4;

        if (top + offset + sizeof (uintptr_t) > bottom)
            {
                if (m_nCurrentCol)
                    FlushBuffer ();

                return false;
            }

        if (++m_nCurrentCol >= TOTAL_COLS)
            {
                m_nCurrentCol = 0;
                FlushBuffer ();

                sprintf (m_sLineBuffer, "+0x%03x: ", offset);
            }

        sprintf (m_sLineBuffer + strlen (m_sLineBuffer), "%016llx ",
                 *(uint64_t *) (top + offset));
        offset += sizeof (uintptr_t);

        return true;
    }

    /*******************************************************/
    bool
    CheckIsPointerValid (uintptr_t ptr)
    {
        auto mbi = VirtualQueryWrapper(ptr);
        return (mbi.State & MEM_COMMIT)
               && (mbi.Protect
                   & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE
                      | PAGE_READWRITE | PAGE_READONLY));
    }
    
    /*******************************************************/
    void
    OnException (_EXCEPTION_POINTERS *ep) override
    {
        Logger::LogMessage ("%s", GetHandlerName());
        
        // Validate RSP and check permissions
        if (!(ep->ContextRecord->ContextFlags & CONTEXT_CONTROL) ||
            !CheckIsPointerValid(ep->ContextRecord->Rsp))
            return;

        auto [base, top, bottom] = GetStackBounds (ep->ContextRecord->Rsp);

        uint32_t offset = 0;
        while (PrintStackWord (top, offset, bottom))
            ;
        
        Rainbomizer::Logger::LogMessage (
            "Base: %016llx, Top: %016llx, Bottom: %016llx", base, top, bottom);
    }

    /*******************************************************/
    const char *
    GetHandlerName () override
    {
        return "Stack";
    }
};

REGISTER_HANDLER (ExceptionHandler_Stack);
}; // namespace Rainbomizer
