
#pragma once

#include <cstdint>

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
    uint32_t field_0x50;
    uint32_t field_0x54;
    uint32_t field_0x58;
    uint32_t field_0x5c;
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
        return *(T *) &(*m_pCodeBlocks)[offset / 0x4000][offset % PAGE_SIZE];
    }

    inline int
    GetTotalPages (uint32_t size)
    {
        return size / PAGE_SIZE + 1;
    }

    inline int
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

    
    static scrProgram* FindProgramByHash (uint32_t hash);
    
};

class scrThread
{
public:
    void *           vft;
    scrThreadContext m_Context;
    uint32_t *       m_pStack;
    uint8_t          field_0xb8[24];
    char             m_szScriptName[64]; //TODO: Move to GtaThread

    static scrThread** sm_pActiveThread;

    static inline scrThread *&
    GetActiveThread ()
    {
        return *sm_pActiveThread;
    }

    scrProgram* GetProgram ();

    static uint16_t FindInstSize (scrProgram *program, uint32_t offset);
    static char *DisassemblInsn (char *out, scrProgram *program,
                                 uint32_t offset, uint32_t bufferLimit = 1024);

    static void InitialisePatterns ();
};
