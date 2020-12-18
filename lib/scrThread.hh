
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

class scrThreadContext
{
public:
    uint32_t m_nThreadId;
    uint32_t m_nScriptHash;
    eScriptState m_nState;
    uint32_t m_nIp;
    uint32_t m_nFrameSP;
    uint32_t m_nSP;
    uint8_t  field_0x18[8];
    float    m_fWaitTime;
    uint8_t  field_0x24[44];
    uint32_t m_nStackSize;
    uint32_t m_nTimerA;
    uint32_t m_nTimerB;
    uint32_t m_nTimerC;
    uint8_t  field_0x60[72];
};

struct scrProgram
{
    static const uint32_t PAGE_SIZE = 0x4000;

    using scrPage = uint8_t[PAGE_SIZE];

    void *    vft;
    void *    m_pPageMap;
    scrPage **m_pCodeBlocks;
    uint32_t  m_nGlobalsSignature;
    uint32_t  m_nCodeSize;
    uint32_t  m_nParameterCount;
    uint32_t  m_nStaticCount;
    uint32_t  m_nGlobalCount;
    uint32_t  m_nNativesCount;
    void *    m_pStaticsPointer;
    void *    m_pGlobalsPointer;
    void *    m_pNativesPointer;
    uint64_t  field_0x48;
    uint64_t  field_0x50;
    uint32_t  m_nScriptHash;
    int32_t   field_0x5c;
    void *    m_pScriptNamePointer;
    scrPage **m_pStringBlocks;
    uint32_t  m_nStringSize;
    int32_t   field_0x74;
    int32_t   field_0x78;
    int32_t   field_0x7c;

    template <typename T>
    T &
    GetCodeByte (uint32_t offset)
    {
        return *(T *) &(*m_pCodeBlocks)[offset / PAGE_SIZE][offset % PAGE_SIZE];
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
    bool InitNativesTable ();
};

class scrThread
{
public:
    void *           vft;
    scrThreadContext m_Context;
    uint64_t *       m_pStack;
    uint8_t          field_0xb8[24];
    char             m_szScriptName[64]; // TODO: Move to GtaThread

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
        Info &operator= (Info const &) = delete;

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

        template <typename T> void PushArg (T arg)
        {
            GetArg<T> (m_Argc++) = arg;
        }
        
        template <typename... Args> Info (Args... args)
        {
            m_Ret  = (void **) &m_StackSpace[MAX_PARAMS - 1];
            m_Args = (void **) &m_StackSpace[0];
            (..., PushArg (args));
        }
    };
    
    static scrThread **      sm_pActiveThread;
    static inline uint64_t **sm_pGlobals;

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
        return *sm_pGlobals;
    }

    template <typename T = uint64_t>
    static inline T &
    GetGlobal (uint32_t index)
    {
        return *(T *) (&GetGlobals ()[index]);
    }

    static inline scrThread *&
    GetActiveThread ()
    {
        return *sm_pActiveThread;
    }

    scrProgram *GetProgram ();

    static uint16_t    FindInstSize (scrProgram *program, uint32_t offset);
    static std::string DisassemblInsn (scrProgram *program, uint32_t offset,
                                       uint32_t bufferLimit = 1024);

    std::pair<uint32_t, uint32_t> FindCurrentFunctionBounds (scrProgram *program
                                                             = nullptr);

    static void InitialisePatterns ();
};
