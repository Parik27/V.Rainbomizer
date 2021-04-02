#include "scrThread.hh"
#include "Patterns/Patterns.hh"
#include "Utils.hh"
#include <utility>
#include <array>
#include <sstream>

#ifdef ENABLE_DEBUG_SERVER
#include <fmt/core.h>
#endif

class gBaseScriptDirectory;

// Instruction set for disassembly later :P
std::array<std::pair<const char *, const char *>, 127> mOpcodes
    = {{{"NOP", ""},
        {"IADD", ""},
        {"ISUB", ""},
        {"IMUL", ""},
        {"IDIV", ""},
        {"IMOD", ""},
        {"INOT", ""},
        {"INEG", ""},
        {"IEQ", ""},
        {"INE", ""},
        {"IGT", ""},
        {"IGE", ""},
        {"ILT", ""},
        {"ILE", ""},
        {"FADD", ""},
        {"FSUB", ""},
        {"FMUL", ""},
        {"FDIV", ""},
        {"FMOD", ""},
        {"FNEG", ""},
        {"FEQ", ""},
        {"FNE", ""},
        {"FGT", ""},
        {"FGE", ""},
        {"FLT", ""},
        {"FLE", ""},
        {"VADD", ""},
        {"VSUB", ""},
        {"VMUL", ""},
        {"VDIV", ""},
        {"VNEG", ""},
        {"IAND", ""},
        {"IOR", ""},
        {"IXOR", ""},
        {"I2F", ""},
        {"F2I", ""},
        {"F2V", ""},
        {"PUSH_CONST_U8", "b"},
        {"PUSH_CONST_U8_U8", "bb"},
        {"PUSH_CONST_U8_U8_U8", "bbb"},
        {"PUSH_CONST_U32", "d"},
        {"PUSH_CONST_F", "f"},
        {"DUP", ""},
        {"DROP", ""},
        {"NATIVE", "bbb"},
        {"ENTER", "bs$"},
        {"LEAVE", "bb"},
        {"LOAD", ""},
        {"STORE", ""},
        {"STORE_REV", ""},
        {"LOAD_N", ""},
        {"STORE_N", ""},
        {"ARRAY_U8", "b"},
        {"ARRAY_U8_LOAD", "b"},
        {"ARRAY_U8_STORE", "b"},
        {"LOCAL_U8", "b"},
        {"LOCAL_U8_LOAD", "b"},
        {"LOCAL_U8_STORE", "b"},
        {"STATIC_U8", "b"},
        {"STATIC_U8_LOAD", "b"},
        {"STATIC_U8_STORE", "b"},
        {"IADD_U8", "b"},
        {"IMUL_U8", "b"},
        {"IOFFSET", ""},
        {"IOFFSET_U8", "b"},
        {"IOFFSET_U8_LOAD", "b"},
        {"IOFFSET_U8_STORE", "b"},
        {"PUSH_CONST_S16", "s"},
        {"IADD_S16", "s"},
        {"IMUL_S16", "s"},
        {"IOFFSET_S16", "s"},
        {"IOFFSET_S16_LOAD", "s"},
        {"IOFFSET_S16_STORE", "s"},
        {"ARRAY_U16", "h"},
        {"ARRAY_U16_LOAD", "h"},
        {"ARRAY_U16_STORE", "h"},
        {"LOCAL_U16", "h"},
        {"LOCAL_U16_LOAD", "h"},
        {"LOCAL_U16_STORE", "h"},
        {"STATIC_U16", "h"},
        {"STATIC_U16_LOAD", "h"},
        {"STATIC_U16_STORE", "h"},
        {"GLOBAL_U16", "h"},
        {"GLOBAL_U16_LOAD", "h"},
        {"GLOBAL_U16_STORE", "h"},
        {"J", "R"},
        {"JZ", "R"},
        {"IEQ_JZ", "R"},
        {"INE_JZ", "R"},
        {"IGT_JZ", "R"},
        {"IGE_JZ", "R"},
        {"ILT_JZ", "R"},
        {"ILE_JZ", "R"},
        {"CALL", "a"},
        {"GLOBAL_U24", "a"},
        {"GLOBAL_U24_LOAD", "a"},
        {"GLOBAL_U24_STORE", "a"},
        {"PUSH_CONST_U24", "a"},
        {"SWITCH", "S"},
        {"STRING", ""},
        {"STRINGHASH", ""},
        {"TEXT_LABEL_ASSIGN_STRING", "b"},
        {"TEXT_LABEL_ASSIGN_INT", "b"},
        {"TEXT_LABEL_APPEND_STRING", "b"},
        {"TEXT_LABEL_APPEND_INT", "b"},
        {"TEXT_LABEL_COPY", ""},
        {"CATCH", ""},
        {"THROW", ""},
        {"CALLINDIRECT", ""},
        {"PUSH_CONST_M1", ""},
        {"PUSH_CONST_0", ""},
        {"PUSH_CONST_1", ""},
        {"PUSH_CONST_2", ""},
        {"PUSH_CONST_3", ""},
        {"PUSH_CONST_4", ""},
        {"PUSH_CONST_5", ""},
        {"PUSH_CONST_6", ""},
        {"PUSH_CONST_7", ""},
        {"PUSH_CONST_FM1", ""},
        {"PUSH_CONST_F0", ""},
        {"PUSH_CONST_F1", ""},
        {"PUSH_CONST_F2", ""},
        {"PUSH_CONST_F3", ""},
        {"PUSH_CONST_F4", ""},
        {"PUSH_CONST_F5", ""},
        {"PUSH_CONST_F6", ""},
        {"PUSH_CONST_F7", ""}}};

gBaseScriptDirectory *scrProgramDirectory;
scrProgram *(*scrProgramRegistry__FindProgramByHash) (gBaseScriptDirectory *,
                                                      uint32_t);
bool (*scrProgram_InitNativesTable) (scrProgram *);

/*******************************************************/
scrProgram *
scrProgram::FindProgramByHash (uint32_t hash)
{
    return scrProgramRegistry__FindProgramByHash (scrProgramDirectory, hash);
}

/*******************************************************/
bool
scrProgram::InitNativesTable ()
{
    return scrProgram_InitNativesTable (this);
}

/*******************************************************/
scrProgram *
scrThread::GetProgram ()
{
    return scrProgram::FindProgramByHash (this->m_Context.m_nScriptHash);
}

/*******************************************************/
std::pair<uint32_t, uint32_t>
scrThread::FindCurrentFunctionBounds (scrProgram *program)
{
    const int ENTER_OPCODE = 45;
    const int LEAVE_OPCODE = 46;

    uint32_t start = 0, end = program->m_nCodeSize;

    for (uint32_t ip = 0; ip < program->m_nCodeSize;
         ip += FindInstSize (program, ip))
        {
            uint8_t opcode = program->GetCodeByte<uint8_t> (ip);

            if (opcode == ENTER_OPCODE)
                start = ip;

            else if (opcode == LEAVE_OPCODE)
                {
                    end = ip;
                    if (ip >= this->m_Context.m_nIp)
                        break;
                }
        }

    return {start, end};
}

/*******************************************************/
uint16_t
scrThread::FindInstSize (scrProgram *program, uint32_t offset)
{
    auto getByteAt = [program] (uint32_t offset) -> uint8_t & {
        return program->GetCodeByte<uint8_t> (offset);
    };

    uint8_t  opcode = getByteAt (offset);
    uint16_t size   = 1;

    auto params = mOpcodes[opcode].second;
    for (size_t i = 0; i < strlen (params); ++i)
        {
            switch (params[i])
                {
                case '$': size += getByteAt (offset + size) + 1; break;
                case 'R': size += 2; break;
                case 'S': size += getByteAt (offset + size) * 6 + 1; break;
                case 'a': size += 3; break;
                case 'b': size += 1; break;
                case 'd':
                case 'f': size += 4; break;
                case 'h':
                case 's': size += 2; break;
                }
        }

    return size;
}

#ifdef ENABLE_DEBUG_SERVER
/*******************************************************/
std::string
scrThread::DisassemblInsn (scrProgram *program, uint32_t offset)
{
    std::string out;

    // Helper functions to get values to facilitate disassembly
    auto getByteAt = [program] (uint32_t offset) -> uint8_t & {
        return program->GetCodeByte<uint8_t> (offset);
    };
    auto getWordAt = [program] (uint32_t offset) -> uint16_t & {
        return program->GetCodeByte<uint16_t> (offset);
    };
    auto getDwordAt = [program] (uint32_t offset) -> uint32_t & {
        return program->GetCodeByte<uint32_t> (offset);
    };
    auto getFloatAt = [program] (uint32_t offset) -> float & {
        return program->GetCodeByte<float> (offset);
    };

    uint8_t opcode = getByteAt (offset++);
    out += fmt::format ("{:06} : {}", offset, mOpcodes[opcode].first);

    auto params = mOpcodes[opcode].second;
    for (size_t i = 0; i < strlen (params); i++)
        {
            switch (params[i])
                {
                    // String
                    case '$': {
                        uint8_t strSize = getByteAt (offset++);
                        offset += strSize;
                        break;
                    }

                    // Relative
                    case 'R': {
                        int16_t jOffset = getWordAt (offset);
                        offset += 2;

                        out += fmt::format (" {:06} (%{:+})", offset + jOffset,
                                            jOffset);
                        break;
                    }
                    // Switch
                    case 'S': {
                        uint8_t numBranches = getByteAt (offset++);
                        out += fmt::format (" [{}]", numBranches);

                        for (int i = 0; i < numBranches; i++)
                            {
                                uint32_t id      = getDwordAt (offset);
                                uint16_t jOffset = getWordAt (offset + 4);

                                out += fmt::format (" {}:{:06}", id, jOffset);

                                offset += 6;
                            }
                        break;
                    }

                    // Address?
                    case 'a': {
                        out += fmt::format (" {:06}",
                                            getDwordAt (offset) & 0xFFFFFF);
                        offset += 3;
                        break;
                    }

                    // Byte immediate
                    case 'b': {
                        out += fmt::format (" {}", getByteAt (offset++));
                        break;
                    }

                    // Integer immediate
                    case 'd': {
                        uint32_t imm32 = getDwordAt (offset);
                        offset += 4;

                        out += fmt::format (" {0}(0x{0:x})", imm32);
                        break;
                    }

                    // Float immediate
                    case 'f': {
                        float imm_f = getFloatAt (offset);
                        offset += 4;

                        out += fmt::format (" {}", imm_f);
                        break;
                    }

                    // Half (ushort)
                    case 'h': {
                        uint16_t imm16 = getWordAt (offset);
                        offset += 2;

                        out += fmt::format (" {}", imm16);
                        break;
                    }

                    // word
                    case 'w': {
                        int16_t imm16 = getWordAt (offset);
                        offset += 2;

                        out += fmt::format (" {}", imm16);
                        break;
                    }
                }
        }

    return out;
}
#endif

/*******************************************************/
eScriptState (*scrThread__Run) (uint64_t *, uint64_t **, scrProgram *,
                                scrThreadContext *);
eScriptState
scrThread::Run (uint64_t *stack, uint64_t **globals, scrProgram *program,
                scrThreadContext *ctx)
{
    return scrThread__Run (stack, globals, program, ctx);
}

/*******************************************************/
void
scrThread::InitialisePatterns ()
{
    sm_pActiveThread = GetRelativeReference<scrThread *> (
        "? 89 2d ? ? ? ? e8 ? ? ? ? ? 8b 8d b0 00 00 00 ? 8d 4d 08", 3, 7);

    // 8b d9 33 d2 ?? 8d 15 ?? ?? ?? ?? ?? 8d 05
    sm_Globals = GetRelativeReference<uint64_t *> (
        "8b d9 33 d2 ? 8d 15 ? ? ? ? ? 8d 05", 14, 18);
    sm_GlobalSizes = GetRelativeReference<uint32_t> (
        "8b d9 33 d2 ? 8d 15 ? ? ? ? ? 8d 05", 7, 11);

    ReadCall (hook::get_pattern (
                  "8d 15 ? ? ? ? ? 8b c0 e8 ? ? ? ? ? 85 ff ? 89 1d", 9),
              scrThread__Run);

    // scrProgram related
    {
        scrProgramDirectory = GetRelativeReference<gBaseScriptDirectory> (
            "? 8d 0d ? ? ? ? ? 89 2d ? ? ? ? e8 ? ? ? ? ? 8b 8d b0 00 00 00 ",
            3, 7);

        ReadCall (hook::get_pattern ("? 8d 0d ? ? ? ? ? 89 2d ? ? ? ? e8 ? ? ? "
                                     "? ? 8b 8d b0 00 00 00",
                                     14),
                  scrProgramRegistry__FindProgramByHash);

        ReadCall (hook::get_pattern (
                      "8b cb e8 ? ? ? ? 8b 43 70 ? 03 c4 a9 00 c0 ff ff", 2),
                  scrProgram_InitNativesTable);
    }
}

scrThread **scrThread::sm_pActiveThread = nullptr;
