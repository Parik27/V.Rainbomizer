#include "missions_YscUtils.hh"
#include <scrThread.hh>

/*******************************************************/
void
YscUtils::FindCodePattern (const std::string &pattern_str,
                           std::function<void (hook::pattern_match)> CB)
{
    m_pProgram->ForEachCodePage ([&] (int, uint8_t *block, size_t size) {
        auto pattern
            = hook::make_range_pattern (uintptr_t (block),
                                        uintptr_t (block) + size, pattern_str);

        pattern.for_each_result (
            [CB] (hook::pattern_match match) { CB (match); });
    });
}

/*******************************************************/
void
YscUtils::FindString (const char *str, void (*CB) (char *))
{
    m_pProgram->ForEachStringPage ([&] (int, uint8_t *block, size_t size) {
        for (uint32_t i = 0; i < size; i++)
            if (strcmp ((char *) block + i, str) == 0)
                {
                    memcpy (block + i, str, 14);
                    break;
                }
    });
}

/*******************************************************/
void
YscUtils::ExtendCode (uint32_t newSize)
{
    // uint32_t newNumPages  = scrProgram::GetTotalPages (newSize);
    // uint32_t origNumPages = scrProgram::GetTotalPages
    // (m_pProgram->m_nCodeSize);

    // if (origNumPages >= newNumPages)
    //     return;

    // m_pProgram->m_nCodeSize = newSize;

    // auto newCodePages = (scrProgram::scrPage**) operator new[] (sizeof
    // (scrProgram::scrPage*) * newNumPages); std::copy_n
    // (m_pProgram->m_pCodeBlocks, origNumPages, newCodePages);

    // for (int i = origNumPages; i < newNumPages; i++)
    //     {
    //         newCodePages[i] = (scrProgram::scrPage *) operator new (
    //             sizeof (scrProgram::scrPage));
    //         memset (newCodePages, 0, sizeof (scrProgram::scrPage));
    //     }

    // operator delete[](m_pProgram->m_pCodeBlocks);
    // m_pProgram->m_pCodeBlocks = newCodePages;
}

/*******************************************************/
uint32_t
YscUtils::GetCodeOffset (uint8_t *code)
{
    uint32_t offset = -1;
    m_pProgram->ForEachCodePage (
        [&] (int blockId, uint8_t *block, size_t size) {
            if (code - block >= 0 && code - block <= scrProgram::PAGE_SIZE)
                offset = blockId * scrProgram::PAGE_SIZE + (code - block);
        });

    return offset;
}
