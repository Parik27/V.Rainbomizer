#include "missions_YscUtils.hh"
#include <scrThread.hh>

#include "common/logger.hh"

/*******************************************************/
void
YscUtils::FindCodePattern (std::string_view pattern_str,
                           std::function<void (hook::pattern_match)> CB,
                           bool                                      translate)
{
    m_pProgram->ForEachCodePage ([&] (int, uint8_t *block, size_t size) {
        auto pattern
            = hook::make_range_pattern (uintptr_t (block),
                                        uintptr_t (block) + size, pattern_str);

        // Translate patterns to new version
        auto& bytes = pattern.getBytes ();
        for (size_t offset = 0; translate && offset < bytes.size (); offset++)
            {
                auto instSize = scrThread::FindInstSize (bytes.data () + offset,
                                                         bytes.size ());

                bytes[offset] = OpCode (YscOpCode (bytes[offset]));

                offset += instSize - 1;
            }

        pattern.for_each_result (
            [CB] (hook::pattern_match match) { CB (match); });
    });
}

/*******************************************************/
void
YscUtils::FindCodePattern (std::string_view               pattern_str,
                           std::function<void (uint32_t)> CB, bool translate)
{
    m_pProgram->ForEachCodePage ([&] (int blockId, uint8_t *block,
                                      size_t size) {
        auto pattern
            = hook::make_range_pattern (uintptr_t (block),
                                        uintptr_t (block) + size, pattern_str);

        // Translate patterns to new version
        auto &bytes = pattern.getBytes ();
        for (size_t offset = 0; translate && offset < bytes.size (); offset++)
            {
                auto instSize = scrThread::FindInstSize (bytes.data () + offset,
                                                         bytes.size ());

                bytes[offset] = OpCode (YscOpCode (bytes[offset]));

                offset += instSize - 1;
            }

        pattern.for_each_result (
            [CB, block, blockId] (hook::pattern_match match) {
                CB (blockId * scrProgram::PAGE_SIZE
                    + (match.get<unsigned char> () - block));
            });
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
            if (code - block >= 0 && code - block <= size)
                offset = blockId * scrProgram::PAGE_SIZE + (code - block);
        });

    return offset;
}
