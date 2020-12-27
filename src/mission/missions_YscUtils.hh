#pragma once

#include <string>
#include <functional>

#include <Utils.hh>
#include <sysMemoryAllocator.hh>
#include <scrThread.hh>

/* Class for working with scrProgram. */
class YscUtils : sysUseAllocator
{
    scrProgram *m_pProgram;

public:
    YscUtils (scrProgram *program) : m_pProgram (program){};

    void FindCodePattern (const std::string &pattern,
                          std::function<void (hook::pattern_match)> CB);

    void FindString (const char *str, void (*CB) (char *));

    void ExtendCode (uint32_t newSize);

    inline scrProgram::scrPage *
    AddNewPage ()
    {
        uint32_t totalPages
            = m_pProgram->GetTotalPages (m_pProgram->m_nCodeSize);
        ExtendCode (totalPages * scrProgram::PAGE_SIZE);

        return &m_pProgram->m_pCodeBlocks[totalPages];
    }

    uint32_t GetCodeOffset (uint8_t* code);
    
    inline scrProgram *
    GetProgram ()
    {
        return m_pProgram;
    }
};
