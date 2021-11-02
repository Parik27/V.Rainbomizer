#include <common/config.hh>
#include <common/logger.hh>

#include <Utils.hh>
#include <cstdint>

#include <utility>
#include <vector>
#include <array>

#include <CCredits.hh>

class CText;

static std::array g_RainbomizerCreditsList
    = {CCreditItem{eCreditLineType::JOB_BIG, "Rainbomizer"},
       CCreditItem{eCreditLineType::SPACE_BIG},
       CCreditItem{eCreditLineType::JOB_MED, "Lead Developer"},
       CCreditItem{eCreditLineType::NAME_BIG, "Parik"},
       CCreditItem{eCreditLineType::SPACE_MED},
       CCreditItem{eCreditLineType::JOB_MED, "Major Contributors"},
       CCreditItem{eCreditLineType::NAME_BIG, "Fryterp23"},
       CCreditItem{eCreditLineType::NAME_BIG, "123robot"},
       CCreditItem{eCreditLineType::SPACE_MED},
       CCreditItem{eCreditLineType::JOB_MED, "Beta Testers"},
       CCreditItem{eCreditLineType::NAME_BIG, "Gibstack"},
       CCreditItem{eCreditLineType::NAME_BIG, "Hugo_One"},
       CCreditItem{eCreditLineType::NAME_BIG, "mo_xi"},
       CCreditItem{eCreditLineType::NAME_BIG, "SpeedyFolf"},
       CCreditItem{eCreditLineType::SPACE_MED},
       CCreditItem{eCreditLineType::SPACE_BIG}};

class RainbomizerCredits
{
    class NewCredits
    {
        CCreditItem *            pOrigArrData = 0;
        uint32_t                 nOrigArrSize = 0;
        uint32_t                 nOrigArrCapa = 0;
        std::vector<CCreditItem> aNewArray;

    public:
        NewCredits (){};

        bool
        ShouldRegenerate ()
        {
            return nOrigArrSize != CCreditArray::sm_Instance->CreditItems.Size;
        }

        void
        Regenerate ()
        {
            if (!ShouldRegenerate ())
                return;

            aNewArray.clear ();

            if (CCreditArray::sm_Instance->CreditItems.Size == 0)
                return;

            for (const auto &i : g_RainbomizerCreditsList)
                aNewArray.push_back (i);

            for (const auto &i : CCreditArray::sm_Instance->CreditItems)
                aNewArray.push_back (i);
        }

        void
        Replace ()
        {
            auto &arr = CCreditArray::sm_Instance->CreditItems;

            pOrigArrData = std::exchange (arr.Data, aNewArray.data ());
            nOrigArrSize = std::exchange (arr.Size, aNewArray.size ());
            nOrigArrCapa = std::exchange (arr.Capacity, arr.Size);
        }

        void
        Restore ()
        {
            auto &arr    = CCreditArray::sm_Instance->CreditItems;
            arr.Data     = pOrigArrData;
            arr.Size     = nOrigArrSize;
            arr.Capacity = nOrigArrCapa;
        }
    };

    inline static NewCredits sm_NewCredits;

    template <auto &EnterCriticalSection_92>
    static void
    ReplaceCreditsArray (void *section)
    {
        EnterCriticalSection_92 (section);

        sm_NewCredits.Regenerate ();
        sm_NewCredits.Replace ();
    }

    template <auto &LeaveCriticalSection_93>
    static void
    RestoreCreditsArray (void *section)
    {
        sm_NewCredits.Restore ();
        LeaveCriticalSection_93 (section);
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
    InitialiseFixJobTitleHooks ()
    {
        hook::pattern p ("74 ? ? 8b 54 03 08 eb ? ? 8d 15 ? ? ? ? ? 8d 0d ? ? "
                         "? ? e8 ? ? ? ?");

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

        REGISTER_HOOK ("8a d9 0f 29 ? ? ? 8d 0d ? ? ? ? 0f 29 ? ? e8 ? ? ? ?",
                       17, ReplaceCreditsArray, void, void *);

        InitialiseFixJobTitleHooks ();

        REGISTER_HOOK_JMP (
            "8d 0d ? ? ? ? ? 8d ? ? ? 01 00 00 ? 8b ? ? 41 0f 28 ? ? 41 0f 28 "
            "? ? ? 8b e3 41 5f 41 5e 41 5d 41 5c 5f 5e 5d e9",
            42, RestoreCreditsArray, void, void *);
    }
} _rbcredits;
