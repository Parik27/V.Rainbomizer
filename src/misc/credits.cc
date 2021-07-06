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
       CCreditItem{eCreditLineType::NAME_BIG, "SpeedyFolf"},
       CCreditItem{eCreditLineType::NAME_BIG, "Parik"},
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
        Rainbomizer::Logger::LogMessage (label);

        char *ret = CText__GetText (tex, label);
        return label;
    }

public:
    RainbomizerCredits ()
    {
        if (true)
            return;

        REGISTER_HOOK ("0f 29 ? ? e8 ? ? ? ? 45 33 f6 44 38 35 ? ? ? ? 0f 84",
                       4, ReplaceCreditsArray, void, void *);

        REGISTER_HOOK ("e8 ? ? ? ? eb ? 66 44 39 74 03 10 74 ? ? 8b 44 03 08 "
                       "eb ? ? 8d 05 ? ? ? ? f3 0f 10 0d ? ? ? ? f3 0f 10",
                       0, FixJobTitles, char *, CText *, char *);

        REGISTER_HOOK_JMP (
            "8d 0d ? ? ? ? ? 8d ? ? 20 01 00 00 ? 8b ? ? 41 0f 28 ? ? 41 0f 28 "
            "? ? ? 8b e3 41 5f 41 5e 41 5d 41 5c 5f 5e 5d e9",
            42, RestoreCreditsArray, void, void *);
    }
} _rbcredits;
