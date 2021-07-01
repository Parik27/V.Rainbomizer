#pragma once

#include "rage.hh"
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

struct CCreditItem
{
    eCreditLineType LineType;
    uint8_t         _pad0x8[4];
    atString        cTextId1;
    uint8_t         _pad0x14[4];
    atString        cTextId2;
    uint8_t         _pad0x24[4];

    CCreditItem () = default;

    CCreditItem (eCreditLineType lineType, atString textId1 = "",
                 atString textId2 = "")
        : LineType (lineType), cTextId1 (textId1), cTextId2 (textId2){};
};

class CCreditArray
{
public:
    atArray<CCreditItem> CreditItems;

    inline static CCreditArray *sm_Instance = nullptr;

    static void InitialisePatterns ();
};
