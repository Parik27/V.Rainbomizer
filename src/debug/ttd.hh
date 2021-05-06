#pragma once

#include "Utils.hh"
#include "common/common.hh"
#include "common/logger.hh"
#include "fmt/core.h"
#include "rage.hh"
#include "scrThread.hh"
#include <array>
#include <cstdint>
#include <stdio.h>
#include <time.h>
#include "exceptions/exceptions_Mgr.hh"

using WriteFunction = void (*) (FILE *, uint8_t *, uint64_t *, uint64_t *);

inline void
WriteStackArgs (FILE *file, uint64_t *SP, uint8_t num)
{
    fwrite (&num, 1, 1, file);
    fwrite (SP - (num - 1), 8, num, file);
}

template <uint8_t Stack = 0, uint8_t IpBytes = 0, int8_t... Pointers>
void
BasicOpcodeWrite (FILE *file, uint8_t *bytes, uint64_t *SP, uint64_t *FSP)
{
    fwrite (&bytes, 8, 1, file);
    fwrite (bytes, 1, IpBytes + 1, file);
    fwrite (SP - (Stack - 1), 8, Stack, file);

    (..., fwrite (reinterpret_cast<void *> (SP[-Pointers]), 8, 1, file));
}

inline static void
NativeWriteFunction (FILE *file, uint8_t *bytes, uint64_t *SP, uint64_t *FSP)
{
    BasicOpcodeWrite<0, 4> (file, bytes, SP, FSP);
    WriteStackArgs (file, SP, bytes[1] >> 2);
}

inline static void
EnterWriteFunction (FILE *file, uint8_t *bytes, uint64_t *SP, uint64_t *FSP)
{
    BasicOpcodeWrite<0, 5> (file, bytes, SP, FSP);
    WriteStackArgs (file, SP - 1, bytes[1]);
}

inline static void
LeaveWriteFunction (FILE *file, uint8_t *bytes, uint64_t *SP, uint64_t *FSP)
{
    BasicOpcodeWrite<0, 5> (file, bytes, SP, FSP);

    uint8_t stackOff = bytes[1];
    uint8_t numRets  = bytes[2];
    WriteStackArgs (file, SP, numRets);
}

template <uint32_t Stacks, int32_t... StringIndices>
inline static void
TextLabelFunction (FILE *file, uint8_t *bytes, uint64_t *SP, uint64_t *FSP)
{
    BasicOpcodeWrite<Stacks, 1> (file, bytes, SP, FSP);

    uint8_t bufferSize = bytes[1];
    fwrite (&bufferSize, 1, 1, file);
    (..., fwrite (reinterpret_cast<char *> (SP[-StringIndices]), 1, bufferSize,
                  file));
}

enum class VariableType
{
    ARRAY,
    STATIC,
    LOCAL,
    OFFSET,
    GLOBAL
};

template <VariableType type, uint8_t size, uint8_t stackSize>
inline static void
VarWriteFunction (FILE *file, uint8_t *bytes, uint64_t *SP, uint64_t *FSP)
{
    BasicOpcodeWrite<stackSize, size / 8> (file, bytes, SP, FSP);

    uint32_t imm
        = *reinterpret_cast<uint32_t *> (bytes + 1) & ((1 << size) - 1);
    int16_t   simm16 = *reinterpret_cast<int16_t *> (bytes + 1);
    uint64_t *ptr    = reinterpret_cast<uint64_t *> (SP[0]);

    switch (type)
        {
            // Array
        case VariableType::ARRAY:
            ptr += (imm * (static_cast<uint32_t> (SP[-1]) + 1));
            break;

            // Static
        case VariableType::STATIC:
            ptr = &scrThread::GetActiveThread ()->GetStaticVariable (imm);
            break;

            // Global
        case VariableType::GLOBAL: ptr = &scrThread::GetGlobal (imm); break;
        case VariableType::LOCAL: ptr = (FSP + imm); break;
        case VariableType::OFFSET: ptr += size == 16 ? simm16 : imm;
        }

    fwrite (&ptr, 8, 1, file);
    fwrite (ptr, 8, 1, file);
}

inline void
OffsetWriteFunction (FILE *file, uint8_t *bytes, uint64_t *SP, uint64_t *FSP)
{
    BasicOpcodeWrite<2, 0, 1> (file, bytes, SP, FSP);
    uint64_t *ptr
        = reinterpret_cast<uint64_t *> (SP[0]) + static_cast<uint32_t> (SP[-1]);

    fwrite (&ptr, 8, 1, file);
    fwrite (ptr, 8, 1, file);
}

inline void
Load_N_WriteFunction (FILE *file, uint8_t *bytes, uint64_t *SP, uint64_t *FSP)
{
    BasicOpcodeWrite<2, 0, 0> (file, bytes, SP, FSP);

    uint64_t *ptr      = reinterpret_cast<uint64_t *> (SP[0]);
    uint32_t  numItems = SP[-1];

    fwrite (&numItems, 8, 1, file);
    for (uint64_t i = 0; i < numItems; i++, ptr++)
        {
            fwrite (&ptr, 8, 1, file);
            fwrite (ptr, 8, 1, file);
        }
}

inline void
Store_N_WriteFunction (FILE *file, uint8_t *bytes, uint64_t *SP, uint64_t *FSP)
{
    BasicOpcodeWrite<2, 0, 0> (file, bytes, SP, FSP);
    uint32_t numItems = SP[-1];

    WriteStackArgs (file, SP - 2, numItems);
}

static constexpr std::array<WriteFunction, 127> Opcodes = {{
    BasicOpcodeWrite,                              // NOP
    BasicOpcodeWrite<2>,                           // IADD
    BasicOpcodeWrite<2>,                           // ISUB
    BasicOpcodeWrite<2>,                           // IMUL
    BasicOpcodeWrite<2>,                           // IDIV
    BasicOpcodeWrite<2>,                           // IMOD
    BasicOpcodeWrite<1>,                           // INOT
    BasicOpcodeWrite<1>,                           // INEG
    BasicOpcodeWrite<2>,                           // IEQ
    BasicOpcodeWrite<2>,                           // INE
    BasicOpcodeWrite<2>,                           // IGT
    BasicOpcodeWrite<2>,                           // IGE
    BasicOpcodeWrite<2>,                           // ILT
    BasicOpcodeWrite<2>,                           // ILE
    BasicOpcodeWrite<2>,                           // FADD
    BasicOpcodeWrite<2>,                           // FSUB
    BasicOpcodeWrite<2>,                           // FMUL
    BasicOpcodeWrite<2>,                           // FDIV
    BasicOpcodeWrite<2>,                           // FMOD
    BasicOpcodeWrite<1>,                           // FNEG
    BasicOpcodeWrite<1>,                           // FEQ
    BasicOpcodeWrite<1>,                           // FNE
    BasicOpcodeWrite<1>,                           // FGT
    BasicOpcodeWrite<1>,                           // FGE
    BasicOpcodeWrite<1>,                           // FLT
    BasicOpcodeWrite<1>,                           // FLE
    BasicOpcodeWrite<6>,                           // VADD
    BasicOpcodeWrite<6>,                           // VSUB
    BasicOpcodeWrite<6>,                           // VMUL
    BasicOpcodeWrite<6>,                           // VDIV
    BasicOpcodeWrite<3>,                           // VNEG
    BasicOpcodeWrite<2>,                           // IAND
    BasicOpcodeWrite<2>,                           // IOR
    BasicOpcodeWrite<2>,                           // IXOR
    BasicOpcodeWrite<1>,                           // I2F
    BasicOpcodeWrite<1>,                           // F2I
    BasicOpcodeWrite<1>,                           // F2V
    BasicOpcodeWrite<0, 1>,                        // PUSH_CONST_U8
    BasicOpcodeWrite<0, 2>,                        // PUSH_CONST_U8_U8
    BasicOpcodeWrite<0, 3>,                        // PUSH_CONST_U8_U8_U8
    BasicOpcodeWrite<0, 4>,                        // PUSH_CONST_U32
    BasicOpcodeWrite<0, 4>,                        // PUSH_CONST_F
    BasicOpcodeWrite<1>,                           // DUP
    BasicOpcodeWrite<1>,                           // DROP
    NativeWriteFunction,                           // NATIVE
    EnterWriteFunction,                            // ENTER
    LeaveWriteFunction,                            // LEAVE
    BasicOpcodeWrite<1, 0, 0>,                     // LOAD
    BasicOpcodeWrite<2, 0, 0>,                     // STORE
    BasicOpcodeWrite<2, 0, 1>,                     // STORE_REV
    Load_N_WriteFunction,                          // LOAD_N
    Store_N_WriteFunction,                         // STORE_N
    VarWriteFunction<VariableType::ARRAY, 8, 2>,   // ARRAY_U8
    VarWriteFunction<VariableType::ARRAY, 8, 2>,   // ARRAY_U8_LOAD
    VarWriteFunction<VariableType::ARRAY, 8, 3>,   // ARRAY_U8_STORE
    VarWriteFunction<VariableType::LOCAL, 8, 0>,   // LOCAL_U8
    VarWriteFunction<VariableType::LOCAL, 8, 0>,   // LOCAL_U8_LOAD
    VarWriteFunction<VariableType::LOCAL, 8, 1>,   // LOCAL_U8_STORE
    VarWriteFunction<VariableType::STATIC, 8, 0>,  // STATIC_U8
    VarWriteFunction<VariableType::STATIC, 8, 0>,  // STATIC_U8_LOAD
    VarWriteFunction<VariableType::STATIC, 8, 1>,  // STATIC_U8_STORE
    BasicOpcodeWrite<1, 1>,                        // IADD_U8
    BasicOpcodeWrite<1, 1>,                        // IMUL_U8
    OffsetWriteFunction,                           // IOFFSET
    VarWriteFunction<VariableType::OFFSET, 8, 1>,  // IOFFSET_U8
    VarWriteFunction<VariableType::OFFSET, 8, 1>,  // IOFFSET_U8_LOAD
    VarWriteFunction<VariableType::OFFSET, 8, 2>,  // IOFFSET_U8_STORE
    BasicOpcodeWrite<1, 2>,                        // PUSH_CONST_S16
    BasicOpcodeWrite<1, 2>,                        // IADD_S16
    BasicOpcodeWrite<1, 2>,                        // IMUL_S16
    VarWriteFunction<VariableType::OFFSET, 8, 1>,  // IOFFSET_S16
    VarWriteFunction<VariableType::OFFSET, 8, 1>,  // IOFFSET_S16_LOAD
    VarWriteFunction<VariableType::OFFSET, 8, 2>,  // IOFFSET_S16_STORE
    VarWriteFunction<VariableType::ARRAY, 16, 2>,  // ARRAY_U16
    VarWriteFunction<VariableType::ARRAY, 16, 2>,  // ARRAY_U16_LOAD
    VarWriteFunction<VariableType::ARRAY, 16, 3>,  // ARRAY_U16_STORE
    VarWriteFunction<VariableType::LOCAL, 8, 0>,   // LOCAL_U16
    VarWriteFunction<VariableType::LOCAL, 8, 0>,   // LOCAL_U16_LOAD
    VarWriteFunction<VariableType::LOCAL, 8, 1>,   // LOCAL_U16_STORE
    VarWriteFunction<VariableType::STATIC, 16, 0>, // STATIC_U16
    VarWriteFunction<VariableType::STATIC, 16, 0>, // STATIC_U16_LOAD
    VarWriteFunction<VariableType::STATIC, 16, 1>, // STATIC_U16_STORE
    VarWriteFunction<VariableType::GLOBAL, 16, 0>, // GLOBAL_U16
    VarWriteFunction<VariableType::GLOBAL, 16, 0>, // GLOBAL_U16_LOAD
    VarWriteFunction<VariableType::GLOBAL, 16, 1>, // GLOBAL_U16_STORE
    BasicOpcodeWrite,                              // J
    BasicOpcodeWrite<1>,                           // JZ
    BasicOpcodeWrite<2>,                           // IEQ_JZ
    BasicOpcodeWrite<2>,                           // INE_JZ
    BasicOpcodeWrite<2>,                           // IGT_JZ
    BasicOpcodeWrite<2>,                           // IGE_JZ
    BasicOpcodeWrite<2>,                           // ILT_JZ
    BasicOpcodeWrite<2>,                           // ILE_JZ
    BasicOpcodeWrite,                              // CALL
    VarWriteFunction<VariableType::GLOBAL, 24, 0>, // GLOBAL_U24
    VarWriteFunction<VariableType::GLOBAL, 24, 0>, // GLOBAL_U24_LOAD
    VarWriteFunction<VariableType::GLOBAL, 24, 1>, // GLOBAL_U24_STORE
    BasicOpcodeWrite<0, 3>,                        // PUSH_CONST_U24
    BasicOpcodeWrite<1>,                           // SWITCH
    BasicOpcodeWrite,                              // STRING
    TextLabelFunction<1, 0>,                       // STRINGHASH
    TextLabelFunction<2, 0, 1>,                    // TEXT_LABEL_ASSIGN_STRING
    TextLabelFunction<2, 0>,                       // TEXT_LABEL_ASSIGN_INT
    TextLabelFunction<2, 0, 1>,                    // TEXT_LABEL_APPEND_STRING
    TextLabelFunction<2, 0>,                       // TEXT_LABEL_APPEND_INT
    BasicOpcodeWrite,                              // TEXT_LABEL_COPY
    BasicOpcodeWrite,                              // CATCH
    BasicOpcodeWrite,                              // THROW
    BasicOpcodeWrite<1>,                           // CALLINDIRECT
    BasicOpcodeWrite,                              // PUSH_CONST_M1
    BasicOpcodeWrite,                              // PUSH_CONST_0
    BasicOpcodeWrite,                              // PUSH_CONST_1
    BasicOpcodeWrite,                              // PUSH_CONST_2
    BasicOpcodeWrite,                              // PUSH_CONST_3
    BasicOpcodeWrite,                              // PUSH_CONST_4
    BasicOpcodeWrite,                              // PUSH_CONST_5
    BasicOpcodeWrite,                              // PUSH_CONST_6
    BasicOpcodeWrite,                              // PUSH_CONST_7
    BasicOpcodeWrite,                              // PUSH_CONST_FM1
    BasicOpcodeWrite,                              // PUSH_CONST_F0
    BasicOpcodeWrite,                              // PUSH_CONST_F1
    BasicOpcodeWrite,                              // PUSH_CONST_F2
    BasicOpcodeWrite,                              // PUSH_CONST_F3
    BasicOpcodeWrite,                              // PUSH_CONST_F4
    BasicOpcodeWrite,                              // PUSH_CONST_F5
    BasicOpcodeWrite,                              // PUSH_CONST_F6
    BasicOpcodeWrite,                              // PUSH_CONST_F7
}};

class TTDFile
{
    FILE *m_File = nullptr;
    enum
    {
        PAUSED,
        CAPTURING_ALL,
        CAPTURING_CFLOW,
        CAPTURING_GLOBALS,
        CAPTURING_LOCALS
    } m_eState
        = PAUSED;

    void
    WriteHeader (scrThread *thread, scrProgram *program)
    {
        char MagicNumber[] = "RBTTD";
        fwrite (MagicNumber, 1, sizeof (MagicNumber), m_File);

        fwrite (thread->m_szScriptName, 1, 64, m_File);
        fwrite (&thread->m_Context.m_nThreadId, 4, 1, m_File);
        fwrite (&thread->m_pStack, 8, 1, m_File);

        uint32_t numPages = program->GetTotalPages (program->m_nCodeSize);
        fwrite (&numPages, 4, 1, m_File);
        fwrite (program->m_pCodeBlocks, 8, numPages, m_File);

        fflush (m_File);
    }

    void
    WriteGlobals ()
    {
        const uint8_t NUM_GLOBALS = 64;

        fwrite (scrThread::sm_Globals, 8, NUM_GLOBALS, m_File);
        for (uint8_t i = 0; i < NUM_GLOBALS; i++)
            {
                Rainbomizer::Logger::LogMessage (
                    "Writing globals: %x (size = %x)", scrThread::sm_Globals[i],
                    scrThread::sm_GlobalSizes[i]);

                fwrite (&scrThread::sm_GlobalSizes[i], 4, 1, m_File);
                if (scrThread::sm_Globals[i] && scrThread::sm_GlobalSizes[i])
                    fwrite (scrThread::sm_Globals[i], 1,
                            scrThread::sm_GlobalSizes[i], m_File);
            }
    }

    void
    WriteStack (scrThread *thread)
    {
        fwrite (&thread->m_Context.m_nStackSize, 4, 1, m_File);
        fwrite (thread->m_pStack, 8, thread->m_Context.m_nStackSize, m_File);
    }

    void
    OpenFile (scrThread *thread, scrProgram *program)
    {
        if (m_File)
            return;

        m_File = Rainbomizer::Common::GetRainbomizerFile (
            fmt::format ("{}_{}.{}.ttd", thread->m_szScriptName,
                         thread->m_Context.m_nThreadId, time (nullptr)),
            "wb", "ttd/");

        WriteHeader (thread, program);
        WriteGlobals ();
        WriteStack (thread);
    }

    void
    CloseFile ()
    {
        fclose (m_File);
        m_File = nullptr;
    }

public:
    bool
    StartCapture (scrThread *thread, scrProgram *program, bool CFlowOnly,
                  bool GlobalsOnly, bool LocalsOnly)
    {
        if (m_eState != PAUSED)
            return true;

        Rainbomizer::Logger::LogMessage ("Starting capture thread: %s",
                                         thread->m_szScriptName);

        if (CFlowOnly)
            m_eState = CAPTURING_CFLOW;
        else if (GlobalsOnly)
            m_eState = CAPTURING_GLOBALS;
        else if (LocalsOnly)
            m_eState = CAPTURING_LOCALS;
        else
            m_eState = CAPTURING_ALL;

        OpenFile (thread, program);
        return false;
    }

    void
    StopCapture ()
    {
        m_eState = PAUSED;
        CloseFile ();
    }

    bool
    ShouldWriteOpcode (YscOpCode op)
    {
        //#define ENABLE_TTD_DEBUG_WRITE_OPCODE
#ifdef ENABLE_TTD_DEBUG_WRITE_OPCODE
        Rainbomizer::Logger::LogMessage ("Attempting to write opcode: %d", op);
#endif

        switch (m_eState)
            {
            case CAPTURING_ALL: return true;
            case PAUSED: return false;
            case CAPTURING_CFLOW:
                switch (op)
                    {
                    case NATIVE:
                    case ENTER:
                    case LEAVE:
                    case LOAD:
                    case STORE:
                    case STORE_REV:

                    default: return false;
                    }
            case CAPTURING_GLOBALS:
                switch (op)
                    {
                    default: return false;
                    }
            case CAPTURING_LOCALS:
                switch (op)
                    {
                    default: return false;
                    }
                break;
            }

        return true;
    }

    void
    WriteOpcode (uint8_t *ip, uint64_t *SP, uint64_t *FSP)
    {
        if (ShouldWriteOpcode (static_cast<YscOpCode> (*ip)))
            {
                Opcodes[*ip](m_File, ip, SP, FSP);

#ifdef ENABLE_TTD_DEBUG_WRITE_OPCODE
                Rainbomizer::Logger::LogMessage (
                    "Successfully wrote opcode: %d", *ip);
#endif
            }
    }
};

class TimeTravelDebugInterface
{

    class TTDFileManager
    {
        inline static std::map<uint32_t, TTDFile> m_Files{};

        uint32_t m_NumIterations = 0;

    public:
        TTDFileManager (uint32_t numIterations = 1)
            : m_NumIterations (numIterations)
        {
        }

        ~TTDFileManager ()
        {
            for (auto &i : m_Files)
                i.second.StopCapture ();
        }

        bool
        IsActive ()
        {
            return m_NumIterations != 0 && m_NumIterations-- != 0;
        }

        bool
        Activate (scrProgram *program)
        {
            if (!IsActive ())
                return false;

            auto thread = scrThread::GetActiveThread ();

            m_CurrentFile = &m_Files[thread->m_Context.m_nThreadId];
            m_CurrentFile->StartCapture (thread, program, false, false, false);

            return true;
        }

        void
        SetNumIterations (uint32_t iters)
        {
            this->m_NumIterations = iters;
        }
    };

    inline static std::map<uint32_t, TTDFileManager> m_Files{};
    inline static TTDFile *                          m_CurrentFile = nullptr;

    static std::string
    HandleOneRequest (nlohmann::json j)
    {
        std::string threadName = j.at ("thread");
        uint32_t    iterations = std::stoi (std::string (j.at ("iterations")));
        bool        pause      = j.at ("pause");

        Rainbomizer::Logger::LogMessage ("Capturing thread: %s, Iterations: %d",
                                         threadName.c_str (), iterations);

        m_Files[rage::atStringHash (threadName)].SetNumIterations (
            (pause) ? 0 : iterations);

        return ":)";
    }

    static void
    HandleCaptureRequests (uWS::HttpResponse<false> *res, uWS::HttpRequest *req)
    {
        res->onData ([res, buffer = std::string ("")] (std::string_view data,
                                                       bool last) mutable {
            buffer.append (data.data (), data.length ());

            if (last)
                {
                    res->writeHeader ("Access-Control-Allow-Origin", "*");
                    res->end (HandleOneRequest (nlohmann::json::parse (buffer))
                                  .c_str ());
                }
        });
        res->onAborted ([] () {});
    }

public:
    static void
    ProcessOpcode (uint8_t *ip, uint64_t *SP, uint64_t *FSP)
    {
        if (m_CurrentFile)
            m_CurrentFile->WriteOpcode (ip, SP, FSP);
    }

    static void
    Process (uint64_t *stack, uint64_t *globals, scrProgram *program,
             scrThreadContext *ctx)
    {
        Rainbomizer::ExceptionHandlerMgr::GetInstance ().Init ();

        m_CurrentFile = nullptr;
        if (auto file = LookupMap (m_Files, ctx->m_nScriptHash))
            {
                if (!file->Activate (program))
                    {
                        m_Files.erase (ctx->m_nScriptHash);
                        Rainbomizer::Logger::LogMessage (
                            "Finished tracing thread: %s",
                            scrThread::GetActiveThread ()->m_szScriptName);
                    }
            }
    }

    static void
    Initialise (uWS::App &app)
    {
        app.post ("/ttd/capture/", HandleCaptureRequests);
    }
};
