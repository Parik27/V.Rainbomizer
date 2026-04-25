#pragma once

#include <cstdint>

#include "GameAddress.hh"
#include "common/logger.hh"
#include "memory/MemorySignature.hh"

class MemoryManager
{
    void InitialiseAllPatterns ();
    std::string GetCacheFileName ();

public:

    uintptr_t m_gameTextAddress = 0;
    uintptr_t m_gameTextSize = 0;
    uintptr_t m_gameGpAddress = 0;
    bool     m_initialised = false;

    void Init ();

    uint32_t
    GetGpAddress ()
    {
        return m_gameGpAddress;
    }

    static MemoryManager &
    Get ()
    {
        static constinit MemoryManager s_memoryMan;
        return s_memoryMan;
    }

    template <typename Func>
    inline void
    FindInstruction (uint32_t instr, Func functor)
    {
        for (std::size_t i = m_gameTextAddress;
             i < m_gameTextAddress + m_gameTextSize; i += 4)
            {
                if (ReadMemory<uint32_t> (i) == instr)
                    functor (i);
            }
    }

    template <typename Ret, typename T>
    static Ret &
    ReadMemory (T addr)
    {
        return *reinterpret_cast<Ret *> (addr);
    }

    template <uint32_t N>
    uintptr_t
    SignatureSearch (const MemorySignature::Signature<N> &sig, size_t start,
                     size_t size)
    {
        for (size_t i = start; i < start + size - N; i++)
            {
                bool found = true;

                for (size_t j = 0; j < N; j++)
                    {
                        if ((ReadMemory<uint8_t> (i + j) & ~sig.masks[j])
                            != sig.bytes[j])
                            {
                                found = false;
                                break;
                            }
                    }

                if (found)
                    return i;
            }

        return 0;
    }

    template <uint32_t N>
    inline uintptr_t
    SignatureSearch (const MemorySignature::Signature<N> &sig)
    {
        return SignatureSearch (sig, m_gameTextAddress, m_gameTextSize);
    }
};
