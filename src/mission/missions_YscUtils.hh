#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <functional>

#include <Utils.hh>
#include <scrThread.hh>
#include <string_view>

#include "CTheScripts.hh"
#include "Patterns/Patterns.hh"
#include "common/logger.hh"

#ifdef ENABLE_DEBUG_SERVER
#include "debug/scripts.hh"
#endif

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
#ifdef ENABLE_DEBUG_SERVER
        ScriptDebugInterface::SetOpcodeHookDisabled (true);
#endif

        const int STACK_SIZE = 512;

        // Create dummy stack for creating a context for the function call.
        scrThread  thread;
        scrThread *prevActiveThread
            = std::exchange (scrThread::GetActiveThread (), &thread);
        uint32_t prevActiveThreadIp = prevActiveThread->m_Context.m_nIp;

        static auto stack = std::make_unique<uint64_t[]> (STACK_SIZE);
        memset (stack.get (), 0, STACK_SIZE * 8);
        thread.m_pStack               = stack.get ();
        thread.m_Context.m_nIp        = ip;
        thread.m_Context.m_nStackSize = STACK_SIZE;
        thread.m_Context.m_nState     = eScriptState::RUNNING;

        // Push arguments and return address to the thread stack.
        (..., thread.Push64 (args));
        thread.Push64 (0); // Returning to 0 terminates the script.

        thread.Run (stack.get (), scrThread::sm_Globals, m_pProgram,
                    &thread.m_Context);

        scrThread::GetActiveThread ()     = prevActiveThread;
        prevActiveThread->m_Context.m_nIp = prevActiveThreadIp;

#ifdef ENABLE_DEBUG_SERVER
        ScriptDebugInterface::SetOpcodeHookDisabled (false);
#endif
    }

    /*******************************************************/
    /* Wrapper CallScriptFunction above to call ysc functions*/
    /*******************************************************/
    template <typename... Args> class ScriptFunction
    {
        using GetProgramFunction = uint32_t (*) ();

        uint32_t           ip               = 0;
        uint32_t           ip_OwningProgram = 0;
        std::string        m_Pattern;
        uint32_t           m_nProgramHash   = 0;
        GetProgramFunction m_pProgramGetter = nullptr;
        bool               bInitialised     = false;

        uint32_t
        GetIp ()
        {
            if (bInitialised && ip_OwningProgram == GetProgramHash ())
                return ip;

            ip               = 0;
            ip_OwningProgram = GetProgramHash ();

            YscUtils utils (GetProgram ());
            if (!ip || ip_OwningProgram != GetProgramHash ())
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

        ScriptFunction (std::string_view pattern, GetProgramFunction func)
            : m_Pattern (pattern), m_pProgramGetter (func)
        {
        }

        uint32_t
        GetProgramHash ()
        {
            uint32_t programHash = m_nProgramHash;
            if (m_pProgramGetter)
                programHash = m_pProgramGetter ();

            return programHash;
        }

        scrProgram *
        GetProgram ()
        {
            return scrProgram::FindProgramByHash (GetProgramHash ());
        }

        bool
        ThreadExists ()
        {
            uint32_t programHash = GetProgramHash ();
            for (auto i : *CTheScripts::aThreads)
                {
                    if (i->m_Context.m_nThreadId == 0
                        || i->m_Context.m_nState == eScriptState::KILLED)
                        continue;

                    if (i->m_Context.m_nScriptHash == programHash)
                        return true;
                }
            return false;
        }

        bool
        CanCall (bool checkScript = false)
        {
            if (checkScript)
                return scrThread::GetActiveThread ()
                       && scrThread::GetActiveThread ()->m_Context.m_nScriptHash
                              == GetProgramHash ();

            if (!GetProgram ())
                return false;

            return true;
        }

        bool
        operator() (Args... args)
        {
            if (!GetIp ())
                return false;

            YscUtils utils (GetProgram ());
            utils.CallScriptFunction (GetIp (), args...);

            return true;
        }
    };

    /* Helper function to get a scrProgram hash from any of the given
     * arguments. It chooses one that's accessible at the moment */
    template <uint32_t... Programs>
    static uint32_t
    AnyValidProgram ()
    {
        uint32_t programHash = 0;
        (...,
         (programHash
          = scrProgram::FindProgramByHash (Programs) ? Programs : programHash));

        return programHash;
    }

    enum PatternIdiom
    {
        GLOBAL_U24,
        GLOBAL_U24_IOFFSET_S16
    };

    /*******************************************************/
    /* Wrapper to a global variable. */
    /*******************************************************/
    template <typename T> class ScriptGlobal
    {
        uint32_t nGlobalIdx = 0;

        std::string  m_Pattern;
        uint32_t     m_PatternOffset;
        PatternIdiom m_PatternIdiom = GLOBAL_U24;

        uint32_t m_nProgram;
        T        defVal;

        bool bFailMessagePrinted = false;

    public:
        ScriptGlobal (std::string_view pattern, uint32_t patternOffset,
                      uint32_t program, PatternIdiom patternIdiom = GLOBAL_U24)
            : m_Pattern (pattern), m_PatternOffset (patternOffset),
              m_PatternIdiom (patternIdiom), m_nProgram (program){};

        ScriptGlobal (std::string_view pattern, uint32_t patternOffset,
                      uint32_t program, T defValue)
            : m_Pattern (pattern), m_PatternOffset (patternOffset),
              m_nProgram (program), defVal (defValue){};

        T *
        Get ()
        {
            if (!nGlobalIdx || !scrThread::sm_Globals)
                return nullptr;

            return &scrThread::GetGlobal<T> (nGlobalIdx);
        }

        uint32_t
        GetIndex ()
        {
            return nGlobalIdx;
        }

        T *
        operator-> ()
        {
            return Get ();
        }

        operator T ()
        {
            if (!nGlobalIdx || !scrThread::sm_Globals)
                return defVal;

            return scrThread::GetGlobal<T> (nGlobalIdx);
        }

        void
        Set (T value)
        {
            if (!nGlobalIdx || !scrThread::sm_Globals)
                return;

            scrThread::GetGlobal<T> (nGlobalIdx) = value;
        }

        bool
        Init ()
        {
            if (nGlobalIdx)
                return true;

            scrProgram *program = scrProgram::FindProgramByHash (m_nProgram);
            if (program)
                return Init (program);

            return false;
        }

        bool
        Init (scrProgram *program)
        {
            if (nGlobalIdx)
                return true;

            if (program->m_nScriptHash != m_nProgram)
                return false;

            YscUtils utils (program);
            utils.FindCodePattern (m_Pattern, [&] (hook::pattern_match m) {
                switch (m_PatternIdiom)
                    {
                    case GLOBAL_U24: // GLOBAL_U24_* <imm24>
                        nGlobalIdx
                            = *m.get<uint32_t> (m_PatternOffset) & 0xFFFFFF;
                        break;

                    case GLOBAL_U24_IOFFSET_S16:
                        nGlobalIdx
                            = *m.get<uint32_t> (m_PatternOffset) & 0xFFFFFF;
                        nGlobalIdx += *m.get<int16_t> (m_PatternOffset + 4);
                        break;
                    }
            });

            if (!nGlobalIdx && !bFailMessagePrinted)
                {
                    Rainbomizer::Logger::LogMessage (
                        "Failed to find offset for global variable: %s (+%02d)",
                        m_Pattern.c_str (), m_PatternOffset);
                    bFailMessagePrinted = true;
                }
            else if (nGlobalIdx)
                {
                    Rainbomizer::Logger::LogMessage (
                        "Found offset for global: %s (+%02d), %d",
                        m_Pattern.c_str (), m_PatternOffset, nGlobalIdx);
                }

            return nGlobalIdx;
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

    /* Initialise the UtilsOps to a certain pattern for further operations */
    void
    Init (uint32_t ip)
    {
        uint8_t *ptr   = &GetProgram ()->GetCodeByte<uint8_t> (ip);
        pPatternResult = ptr;
    }

    uint32_t
    GetWorkingIp ()
    {
        if (pPatternResult)
            return GetCodeOffset (pPatternResult);
        return -1u;
    }

    /* Reinitialises the UtilsOps to the branch destination at offset of stored
     * ptr */
    void
    FollowBranchDestination (int64_t offset)
    {
        switch (*Get<YscOpCode> (offset))
            {
                // Absolute offset
            case YscOpCode::CALL:
                pPatternResult = &GetProgram ()->GetCodeByte<uint8_t> (
                    (*Get<uint32_t> (offset) >> 8));
                break;

                // Relative offset
            case YscOpCode::J:
            case YscOpCode::JZ:
            case YscOpCode::IEQ_JZ:
            case YscOpCode::INE_JZ:
            case YscOpCode::IGT_JZ:
            case YscOpCode::IGE_JZ:
            case YscOpCode::ILT_JZ:
            case YscOpCode::ILE_JZ:
                pPatternResult += *Get<uint16_t> (offset + 1) + (offset + 3);
                break;

            default: bOperationFailed = true; break;
            }
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
        memset (pPatternResult + offset, uint8_t (YscOpCode::NOP), size);
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
        memcpy (pPatternResult + offset, &bytes[0], std::size (bytes));
    }

    explicit operator bool () const
    {
        return bOperationFailed || !pPatternResult;
    }
};
