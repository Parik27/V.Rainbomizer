#include "Patterns/Patterns.hh"
#include "atArray.hh"
#include <common/config.hh>
#include <common/logger.hh>
#include <common/minhook.hh>

#include <Utils.hh>
#include <cstdint>

#include <memory>
#include <utility>
#include <vector>
#include <array>

#include <CCredits.hh>

class CText;

struct CreditItem
{
    eCreditLineType type;
    std::string     title;
};

static std::array g_RainbomizerCreditsList
    = {CreditItem{eCreditLineType::JOB_BIG, "Rainbomizer"},
       CreditItem{eCreditLineType::SPACE_BIG},
       CreditItem{eCreditLineType::JOB_MED, "Lead Developer"},
       CreditItem{eCreditLineType::NAME_BIG, "Parik"},
       CreditItem{eCreditLineType::SPACE_MED},
       CreditItem{eCreditLineType::JOB_MED, "Major Contributors"},
       CreditItem{eCreditLineType::NAME_BIG, "Fryterp23"},
       CreditItem{eCreditLineType::NAME_BIG, "123robot"},
       CreditItem{eCreditLineType::SPACE_MED},
       CreditItem{eCreditLineType::JOB_MED, "Beta Testers"},
       CreditItem{eCreditLineType::NAME_BIG, "Gibstack"},
       CreditItem{eCreditLineType::NAME_BIG, "Hugo_One"},
       CreditItem{eCreditLineType::NAME_BIG, "mo_xi"},
       CreditItem{eCreditLineType::NAME_BIG, "SpeedyFolf"},
       CreditItem{eCreditLineType::SPACE_MED},
       CreditItem{eCreditLineType::SPACE_BIG}};

class RainbomizerCredits
{
    class NewCredits
    {
        uint8_t *pOrigArrData = 0;
        uint32_t nOrigArrSize = 0;
        uint32_t nOrigArrCapa = 0;

        std::unique_ptr<uint8_t[]> pCreditItemData;
        atArrayGetSizeWrapper<CCreditItem> aEditedCredits;

        auto GetCreditItems ()
        {
            return CCreditArray::sm_Instance->GetCreditItems();
        }

    public:
        NewCredits (){};

        bool
        ShouldRegenerate ()
        {
            return nOrigArrSize != GetCreditItems ().Size;
        }

        size_t
        GetTotalCreditLinesAfterEditing ()
        {
            return g_RainbomizerCreditsList.size () + GetCreditItems ().Size;
        }

        void
        ResetEditedCreditsData (size_t size)
        {
            pCreditItemData
                = std::make_unique<uint8_t[]> (size * CCreditItem::GetSize ());

            aEditedCredits
                = decltype (aEditedCredits) (pCreditItemData.get (), size);
        }

        void
        Regenerate ()
        {
            if (!ShouldRegenerate ())
                return;

            if (GetCreditItems().Size == 0)
                return;

            ResetEditedCreditsData (GetTotalCreditLinesAfterEditing ());
            auto &arr = aEditedCredits;

            size_t idx = 0;
            for (const auto &i : g_RainbomizerCreditsList)
                {
                    arr[idx] = CCreditArray::sm_Instance->GetCreditItems ()[3];

                    arr[idx].Get<eCreditLineType> ("LineType"_joaat) = i.type;

                    if (Rainbomizer::Logger::GetGameBuild () >= 2802)
                        arr[idx].Get<bool> ("bLiteral"_joaat) = true;

                    arr[idx].Get<atString> ("cTextId1"_joaat)
                        = i.title.c_str ();

                    idx++;
                }

            for (const auto &i : CCreditArray::sm_Instance->GetCreditItems ())
                {
                    arr[idx] = i;
                    idx++;
                }
        }

        void
        Replace ()
        {
            auto &arr = CCreditArray::sm_Instance->GetCreditItems();

            pOrigArrData
                = std::exchange (*reinterpret_cast<uint8_t **> (&arr.Data),
                                 pCreditItemData.get ());

            nOrigArrSize = std::exchange (arr.Size, aEditedCredits.Size);
            nOrigArrCapa = std::exchange (arr.Capacity, arr.Size);
        }

        void
        Restore ()
        {
            auto &arr    = CCreditArray::sm_Instance->GetCreditItems();

            *reinterpret_cast<uint8_t **> (&arr.Data) = pOrigArrData;
            arr.Size                                  = nOrigArrSize;
            arr.Capacity                              = nOrigArrCapa;
        }
    };

    inline static NewCredits sm_NewCredits;

    template <auto &CCredits__Draw>
    static void
    ReplaceCreditsArray (bool a1)
    {
        sm_NewCredits.Regenerate ();
        sm_NewCredits.Replace ();

        CCredits__Draw (a1);

        sm_NewCredits.Restore ();
    }

    template <auto &CText__GetText>
    static char *
    FixJobTitles (CText *tex, char *label)
    {
        char *ret = CText__GetText (tex, label);

        if (!ret || ret[0] == '\0')
            return label;

        return ret;
    }

    void
    InitialiseFixJobTitleHooks (std::string_view pattern)
    {
        hook::pattern p (pattern);

        using CText_GetText_Prototype = char *(*) (CText *, char *);
        static CText_GetText_Prototype orig;

        p.for_each_result ([] (hook::pattern_match res) {
            RegisterHook (res.get<void> (23), orig, FixJobTitles<orig>);
        });
    }

public:

    RainbomizerCredits ()
    {
        if (!ConfigManager::ReadConfig ("RainbomizerCredits"))
            return;

        InitialiseAllComponents ();

        REGISTER_MH_HOOK_OPERAND (
            "? 8d ? ? ? ? ? 33 ? ? 88 ? ? e8 ? ? ? ? ? 8d ? ? ? ? "
            "? 33 ? e8 ? ? ? ? ? 8d ? ? ? 8d ? ? ? ? ?",
            3, ReplaceCreditsArray, void, bool);

        InitialiseFixJobTitleHooks (
            "74 ? ? 8b 54 18 28 eb ? ? 8d 15 ? ? ? ? ? 8d 0d ? ? ? ? e8 ? ? ? "
            "? eb ? 66 39 ? ? ? ");

        InitialiseFixJobTitleHooks (
            "74 ? ? 8b 54 03 08 eb ? ? 8d 15 ? ? ? ? ? 8d 0d ? ? "
            "? ? e8 ? ? ? ?");
    }
} _rbcredits;
