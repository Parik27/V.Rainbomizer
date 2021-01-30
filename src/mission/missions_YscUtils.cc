#include "missions_YscUtils.hh"
#include <scrThread.hh>

/*******************************************************/
void
YscUtils::FindCodePattern (std::string_view pattern_str,
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
