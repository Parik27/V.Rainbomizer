
#pragma once

#include <cstdint>

class scrThreadContext
{
public:
    uint32_t m_nThreadId;
    uint32_t m_nScriptHash;
    uint32_t m_nState;
    uint32_t m_nIp;
    uint32_t m_nFrameSP;
    uint32_t m_nSP;
    uint8_t  field_0x18[56];
    uint32_t field_0x50;
    uint32_t field_0x54;
    uint32_t field_0x58;
    uint32_t field_0x5c;
    uint8_t  field_0x60[72];
};

class scrThread
{
public:
    void *           vft;
    scrThreadContext m_Context;
    uint32_t *       m_pStack;
    uint8_t          field_0xb8[24];

    static scrThread** sm_pActiveThread;

    static inline scrThread *&
    GetActiveThread ()
    {
        return *sm_pActiveThread;
    }

    static void InitialisePatterns ();
};
