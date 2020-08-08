#include "CText.hh"
#include "Utils.hh"

//?? 8d 0d ?? ?? ?? ?? ?? 8b c5 ?? 8b d3 e8 ?? ?? ?? ?? ?? 8b d7 33 c9

//?? 63 c2 b0 01 ?? 38 84 ?? ?? ?? ?? ?? 74 ?? ?? 38 84 ?? ?? ?? ?? ?? 74 ?? 

char* (*CText__GetText) (CText *, uint32_t);
bool (*CText__HasThisAdditionalTextLoaded) (CText *, const char *, uint32_t);
void (*CText__RequestAdditionalText) (CText *, uint32_t, const char *, bool,
                                      uint32_t);

/*******************************************************/
char *
CText::GetText (uint32_t hash)
{
    return CText__GetText (this, hash);
}

/*******************************************************/
bool
CText::HasThisAdditionalTextLoaded (const char *gxt, int slot)
{
    return CText__HasThisAdditionalTextLoaded (this, gxt, slot);
}

/*******************************************************/
void
CText::RequestAdditionalText (uint32_t slot, const char *gxt, bool sync,
                              int dlc)
{
    CText__RequestAdditionalText (this, slot, gxt, sync, dlc);
}

/*******************************************************/
void
CText::InitialisePatterns ()
{
    ConvertCall (
        hook::get_pattern (
            "? 89 5c ? ? ? 89 6c ? ? 89 54 ? ? 56 57 ? 56 ? 83 ec 20 "),
        CText__GetText);

    auto match = hook::pattern ("? 8d 0d ? ? ? ? ? 8b c5 ? 8b d3 e8 ? "
                                "? ? ? ? 8b d7 33 c9 ")
                     .get_one ();

    TheText = GetRelativeReference<CText> (
        "? 8d 0d ? ? ? ? ? 8b c5 ? 8b d3 e8 ? ? ? ? ? 8b d7 33 c9", 3, 7);

    ReadCall (match.get<void> (-17), CText__HasThisAdditionalTextLoaded);

    ReadCall (hook::get_pattern (
                  "c7 44 ? ? 01 00 00 00 ? 33 c9 e8 ? ? ? ? ? 83 c4 38", 11),
              CText__RequestAdditionalText);
    
}

CText* CText::TheText;
