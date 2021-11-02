
#pragma once

#include <cstdint>
#include <string>

enum class eScriptState : uint32_t
{
    RUNNING = 0,
    WAITING,
    KILLED,
    STATE_3,
    STATE_4
};

enum YscOpCode : uint8_t
{
    NOP,
    IADD,
    ISUB,
    IMUL,
    IDIV,
    IMOD,
    INOT,
    INEG,
    IEQ,
    INE,
    IGT,
    IGE,
    ILT,
    ILE,
    FADD,
    FSUB,
    FMUL,
    FDIV,
    FMOD,
    FNEG,
    FEQ,
    FNE,
    FGT,
    FGE,
    FLT,
    FLE,
    VADD,
    VSUB,
    VMUL,
    VDIV,
    VNEG,
    IAND,
    IOR,
    IXOR,
    I2F,
    F2I,
    F2V,
    PUSH_CONST_U8,
    PUSH_CONST_U8_U8,
    PUSH_CONST_U8_U8_U8,
    PUSH_CONST_U32,
    PUSH_CONST_F,
    DUP,
    DROP,
    NATIVE,
    ENTER,
    LEAVE,
    LOAD,
    STORE,
    STORE_REV,
    LOAD_N,
    STORE_N,
    ARRAY_U8,
    ARRAY_U8_LOAD,
    ARRAY_U8_STORE,
    LOCAL_U8,
    LOCAL_U8_LOAD,
    LOCAL_U8_STORE,
    STATIC_U8,
    STATIC_U8_LOAD,
    STATIC_U8_STORE,
    IADD_U8,
    IMUL_U8,
    IOFFSET,
    IOFFSET_U8,
    IOFFSET_U8_LOAD,
    IOFFSET_U8_STORE,
    PUSH_CONST_S16,
    IADD_S16,
    IMUL_S16,
    IOFFSET_S16,
    IOFFSET_S16_LOAD,
    IOFFSET_S16_STORE,
    ARRAY_U16,
    ARRAY_U16_LOAD,
    ARRAY_U16_STORE,
    LOCAL_U16,
    LOCAL_U16_LOAD,
    LOCAL_U16_STORE,
    STATIC_U16,
    STATIC_U16_LOAD,
    STATIC_U16_STORE,
    GLOBAL_U16,
    GLOBAL_U16_LOAD,
    GLOBAL_U16_STORE,
    J,
    JZ,
    IEQ_JZ,
    INE_JZ,
    IGT_JZ,
    IGE_JZ,
    ILT_JZ,
    ILE_JZ,
    CALL,
    GLOBAL_U24,
    GLOBAL_U24_LOAD,
    GLOBAL_U24_STORE,
    PUSH_CONST_U24,
    SWITCH,
    STRING,
    STRINGHASH,
    TEXT_LABEL_ASSIGN_STRING,
    TEXT_LABEL_ASSIGN_INT,
    TEXT_LABEL_APPEND_STRING,
    TEXT_LABEL_APPEND_INT,
    TEXT_LABEL_COPY,
    CATCH,
    THROW,
    CALLINDIRECT,
    PUSH_CONST_M1,
    PUSH_CONST_0,
    PUSH_CONST_1,
    PUSH_CONST_2,
    PUSH_CONST_3,
    PUSH_CONST_4,
    PUSH_CONST_5,
    PUSH_CONST_6,
    PUSH_CONST_7,
    PUSH_CONST_FM1,
    PUSH_CONST_F0,
    PUSH_CONST_F1,
    PUSH_CONST_F2,
    PUSH_CONST_F3,
    PUSH_CONST_F4,
    PUSH_CONST_F5,
    PUSH_CONST_F6,
    PUSH_CONST_F7
};

class scrThreadContext
{
public:
    uint32_t     m_nThreadId    = 0;
    uint32_t     m_nScriptHash  = 0;
    eScriptState m_nState       = eScriptState::WAITING;
    uint32_t     m_nIp          = 0;
    uint32_t     m_nFrameSP     = 0;
    uint32_t     m_nSP          = 0;
    uint8_t      field_0x18[8]  = {0};
    float        m_fWaitTime    = 0;
    uint8_t      field_0x24[44] = {0};
    uint32_t     m_nStackSize   = 0;
    uint32_t     m_nTimerA      = 0;
    uint32_t     m_nTimerB      = 0;
    uint32_t     m_nTimerC      = 0;
    uint8_t      field_0x60[72] = {0};
};

struct scrProgram
{
    static const uint32_t PAGE_SIZE = 0x4000;

    using scrPage = uint8_t *;

    void *   vft;
    void *   m_pPageMap;
    scrPage *m_pCodeBlocks;
    uint32_t m_nGlobalsSignature;
    uint32_t m_nCodeSize;
    uint32_t m_nParameterCount;
    uint32_t m_nStaticCount;
    uint32_t m_nGlobalCount;
    uint32_t m_nNativesCount;
    void *   m_pStaticsPointer;
    void *   m_pGlobalsPointer;
    void **  m_pNativesPointer;
    uint64_t field_0x48;
    uint64_t field_0x50;
    uint32_t m_nScriptHash;
    int32_t  field_0x5c;
    void *   m_pScriptNamePointer;
    scrPage *m_pStringBlocks;
    uint32_t m_nStringSize;
    int32_t  field_0x74;
    int32_t  field_0x78;
    int32_t  field_0x7c;

    template <typename T>
    T &
    GetCodeByte (uint32_t offset)
    {
        return *(T *) &(m_pCodeBlocks[offset / PAGE_SIZE][offset % PAGE_SIZE]);
    }

    static inline int
    GetTotalPages (uint32_t size)
    {
        return size / PAGE_SIZE + 1;
    }

    static inline int
    GetPageSize (int page, uint32_t size)
    {
        if (page >= GetTotalPages (size))
            return 0;
        if (page >= GetTotalPages (size) - 1)
            return size % PAGE_SIZE;

        return PAGE_SIZE;
    }

    template <typename F>
    void
    ForEachCodePage (F func)
    {
        for (int i = 0; i < GetTotalPages (m_nCodeSize); i++)
            func (i, (uint8_t *) m_pCodeBlocks[i],
                  GetPageSize (i, m_nCodeSize));
    }

    template <typename F>
    void
    ForEachStringPage (F func)
    {
        for (int i = 0; i < GetTotalPages (m_nStringSize); i++)
            func (i, (uint8_t *) m_pStringBlocks[i],
                  GetPageSize (i, m_nStringSize));
    }

    static scrProgram *FindProgramByHash (uint32_t hash);
    bool               InitNativesTable ();
};

class scrThread
{
public:
    void *           vft;
    scrThreadContext m_Context;
    uint64_t *       m_pStack           = nullptr;
    uint8_t          field_0xb8[24]     = {0};
    char             m_szScriptName[64] = {0}; // TODO: Move to GtaThread

    class Info
    {
        void **  m_Ret;
        uint32_t m_Argc;
        void **  m_Args;

        alignas (uintptr_t) uint8_t m_VectorSpace[192];

        // Custom Storage
        inline static const uint32_t MAX_PARAMS = 24;

        uint8_t m_StackSpace[MAX_PARAMS * sizeof (void *)] = {0};

    public:
        Info (Info const &) = delete;

        template <typename T = uint64_t>
        inline T &
        GetArg (std::uint8_t n)
        {
            return *(T *) &m_Args[n];
        }

        template <typename T = uint64_t>
        inline T &
        GetReturn (std::uint8_t n = 0)
        {
            return *(T *) &m_Ret[n];
        }

        template <typename T>
        void
        PushArg (T arg)
        {
            if (m_Argc < MAX_PARAMS)
                GetArg<T> (m_Argc++) = arg;
        }

        template <typename... Args> Info (Args... args)
        {
            m_Ret  = (void **) &m_StackSpace[128];
            m_Args = (void **) &m_StackSpace[0];
            m_Argc = 0;
            (..., PushArg (args));
        }
    };

    static scrThread **      sm_pActiveThread;
    static inline uint64_t **sm_Globals;     // sm_Globals[64]
    static inline uint32_t * sm_GlobalSizes; // sm_GlobalSizes[64]

    static bool
    IsCurrentScriptAddon ()
    {
        if (!scrThread::GetActiveThread ())
            return true;

        return !scrThread::GetActiveThread ()->IsYscScript ();
    }

    inline bool
    IsYscScript ()
    {
        scrProgram *program = GetProgram ();
        if (program)
            return program->m_nCodeSize;
        return false;
    }

    static inline uint64_t *&
    GetGlobals ()
    {
        return *sm_Globals;
    }

    template <typename T = uint64_t>
    static inline T &
    GetGlobal (uint32_t index)
    {
        return *(T *) (&GetGlobals ()[index]);
    }

    template <typename T = uint64_t>
    inline T &
    GetStaticVariable (uint32_t index)
    {
        return *(T *) (&m_pStack[index]);
    }

    template <typename T = uint64_t>
    inline T &
    GetLocalVariable (uint32_t index)
    {
        return *(T *) (&m_pStack[m_Context.m_nFrameSP + index]);
    }

    static inline scrThread *&
    GetActiveThread ()
    {
        return *sm_pActiveThread;
    }

    static bool
    CheckActiveThread (uint32_t hash)
    {
        return GetActiveThread ()
               && GetActiveThread ()->m_Context.m_nScriptHash == hash;
    }

    // Functions for stack manipulation
    template <typename T = uint64_t>
    void
    Push64 (T value)
    {
        m_pStack[m_Context.m_nSP++] = static_cast<uint64_t> (value);
    }

    template <typename T = uint64_t>
    T
    Pop64 ()
    {
        return m_pStack[m_Context.m_nSP--];
    }

    scrProgram *GetProgram ();

    static uint16_t    FindInstSize (scrProgram *program, uint32_t offset);
    static std::string DisassemblInsn (scrProgram *program, uint32_t offset);

    std::pair<uint32_t, uint32_t> FindCurrentFunctionBounds (scrProgram *program
                                                             = nullptr);

    static eScriptState Run (uint64_t *stack, uint64_t **globals,
                             scrProgram *program, scrThreadContext *ctx);

    static void InitialisePatterns ();
};
