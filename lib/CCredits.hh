#pragma once

#include "rage.hh"
#include "ParserUtils.hh"
#include <cstdint>

enum class eCreditLineType : uint32_t
{
    JOB_BIG = 0,
    JOB_MED,
    JOB_SMALL,
    NAME_BIG,
    NAME_MED,
    NAME_SMALL,
    SPACE_BIG,
    SPACE_MED,
    SPACE_SMALL,
    SPACE_END,
    SPRITE_1,
    LEGALS,
    AUDIO_NAME,
    AUDIO_LEGALS,
    JOB_AND_NAME_MED
};

struct CCreditItem : public ParserWrapper<CCreditItem>
{
};

class CCreditArray : public ParserWrapper<CCreditArray>
{
public:
    auto&
    GetCreditItems ()
    {
        return Get<atArrayGetSizeWrapper<CCreditItem>> ("CreditItems"_joaat);
    }

    inline static CCreditArray *sm_Instance = nullptr;

    static void InitialisePatterns ();
};
