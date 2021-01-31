#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <functional>

#include <Utils.hh>
#include <scrThread.hh>
#include <string_view>

#include "Patterns/Patterns.hh"
#include "common/logger.hh"

/* Class for working with scrProgram. */
class YscUtils
{
    scrProgram *m_pProgram;

public:
    YscUtils (scrProgram *program) : m_pProgram (program){};

    void FindCodePattern (std::string_view                          pattern,
                          std::function<void (hook::pattern_match)> CB);

    void FindString (const char *str, void (*CB) (char *));

    template <typename... Args>
    bool
    IsAnyOf (Args... args)
    {
        bool anyOf = false;
        (..., (anyOf = (m_pProgram->m_nScriptHash == args) ? true : anyOf));

        return anyOf;
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

    /*******************************************************/
    /* Wrapper CallScriptFunction above to call ysc functions*/
    /*******************************************************/
    template <typename... Args> class ScriptFunction
    {
        uint32_t    ip = 0;
        std::string m_Pattern;
        uint32_t    m_nProgramHash = 0;
        bool        bInitialised   = false;

        uint32_t
        GetIp ()
        {
            if (bInitialised)
                return ip;

            YscUtils utils (scrProgram::FindProgramByHash (m_nProgramHash));
            if (!ip)
                {
                    utils.FindCodePattern (m_Pattern,
                                           [&] (hook::pattern_match m) {
                                               ip = utils.GetCodeOffset (
                                                   m.get<uint8_t> ());
                                           });
                }

            if (!ip)
                Rainbomizer::Logger::LogMessage (
                    "Failed to find instruction pointer for function %s",
                    m_Pattern.c_str ());

            bInitialised = true;
            return ip;
        }

    public:

        ScriptFunction (std::string_view pattern, uint32_t program)
            : m_Pattern (pattern), m_nProgramHash (program)
        {
        }

        bool
        operator() (Args... args)
        {
            if (!GetIp ())
                return false;

            YscUtils utils (scrProgram::FindProgramByHash (m_nProgramHash));
            utils.CallScriptFunction (GetIp (), args...);

            return true;
        }
    };

    /*******************************************************/
    /* Wrapper to a global variable. */
    /*******************************************************/
    template <typename T> class ScriptGlobal
    {
        uint32_t    nGlobalIdx = 0;

        std::string m_Pattern;
        uint32_t m_PatternOffset;

        uint32_t m_nProgram;
        T        defVal;

        bool bFailMessagePrinted = false;

    public:
        ScriptGlobal (std::string_view pattern, uint32_t patternOffset,
                      uint32_t program)
            : m_Pattern (pattern), m_PatternOffset (patternOffset),
              m_nProgram (program){};

        ScriptGlobal (std::string_view pattern, uint32_t patternOffset,
                      uint32_t program, T defValue)
            : m_Pattern (pattern), m_PatternOffset (patternOffset),
              m_nProgram (program), defVal (defValue){};

        operator T ()
        {
            if (!nGlobalIdx || !scrThread::sm_pGlobals)
                return defVal;

            return scrThread::GetGlobal (nGlobalIdx);
        }

        void
        Init (scrProgram* program)
        {
            if (nGlobalIdx)
                return;

            if (program->m_nScriptHash != m_nProgram)
                return;

            YscUtils utils (program);
            utils.FindCodePattern (m_Pattern, [&] (hook::pattern_match m) {
                // GLOBAL_U24_* <imm24>
                nGlobalIdx = *m.get<uint32_t> (m_PatternOffset) & 0xFFFFFF;
            });

            if (!nGlobalIdx && !bFailMessagePrinted)
                {
                    Rainbomizer::Logger::LogMessage (
                        "Failed to find offset for global variable: %s (+%02d)",
                        m_Pattern.c_str (), m_PatternOffset);
                    bFailMessagePrinted = true;
                }
        }
    };
};

class YscUtilsOps : public YscUtils
{
    bool     bOperationFailed = false;
    uint8_t *pPatternResult   = nullptr;

public:

    using YscUtils::YscUtils;
    
    /* Initialise the UtilsOps to a certain pattern for further operations */
    void
    Init (std::string_view pattern)
    {
        uint8_t *ptr = nullptr;
        FindCodePattern (pattern, [&ptr] (hook::pattern_match m) {
            if (!ptr)
                ptr = m.get<uint8_t> (0);
        });

        pPatternResult = ptr;
    }

    /* Returns the evaluated pattern/initialised value of the stored ptr */
    template <typename T>
    T *
    Get (int64_t offset)
    {
        return reinterpret_cast<T *> (pPatternResult + offset);
    }

    /* Makes a NOP at offset of size */
    void
    NOP (int64_t offset, size_t size)
    {
        if (!pPatternResult)
            return void (bOperationFailed = true);
        memset (pPatternResult + offset, uint8_t(YscOpCode::NOP), size);
    }

    /* Writes a value of type T at offset */
    template <typename T>
    void
    Write (int64_t offset, T value)
    {
        if (!pPatternResult)
            return void (bOperationFailed = true);
        *Get<T> (offset) = value;
    }

    template <typename T>
    void
    WriteBytes (int64_t offset, const T &bytes)
    {        
        if (!pPatternResult)
            return void (bOperationFailed = true);
        memcpy (pPatternResult, &bytes[0], std::size (bytes));
    }

    explicit operator bool const ()
    {
        return bOperationFailed || !pPatternResult;
    }
};
