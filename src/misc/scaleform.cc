#include "CTheScripts.hh"
#include "common/common.hh"
#include "common/config.hh"
#include "common/logger.hh"
#include "rage.hh"
#include "scrThread.hh"
#include <Utils.hh>
#include <array>
#include <cctype>
#include <cstdint>
#include <CScaleformMgr.hh>
#include <string>

using namespace std::string_literals;

class ScaleformRandomizer
{
    /*******************************************************/
    inline static std::vector<std::string> m_MissionFailInsults;
    inline static std::vector<std::string> m_ScaleformStrings;

    // Config
    RB_C_CONFIG_START { int FailMessageOdds = 20; }
    RB_C_CONFIG_END

    /*******************************************************/
    static int
    GetFailMessageStart (std::string &str)
    {
        for (int i = 0; i < str.size (); i++)
            {
                char ch = str[i];
                if (isalpha (ch))
                    return i;

                else if (ch == '<')
                    return str.find ("'>") + 2;
            }

        return 0;
    }

    /*******************************************************/
    static void
    SetMissionFailInsult (ScaleformMethodStruct::ParamStruct &parameter,
                          bool useLastInsult = false)
    {
        ReadInsultsList ();

        if (m_MissionFailInsults.empty ())
            return;

        // Only affect mission failed scaleform.
        if (!scrThread::GetActiveThread ()
            || scrThread::GetActiveThread ()->m_Context.m_nScriptHash
                   != "replay_controller"_joaat)
            return;

        static std::string m_LastInsult = "";

        auto string = parameter.GetStr ();
        if (!string)
            return;

        if (!useLastInsult)
            {
                m_LastInsult = *string;

                m_LastInsult.insert (GetFailMessageStart (m_LastInsult),
                                     GetRandomElement (m_MissionFailInsults));

                // Reset it if odds not right.
                if (RandomInt (100) >= Config ().FailMessageOdds)
                    m_LastInsult = "";
            }

        if (m_LastInsult.empty ())
            return;

        *string = m_LastInsult.data ();
    }

    /*******************************************************/
    static void
    RandomizeScaleformMethod (char *                              name,
                              ScaleformMethodStruct::ParamStruct *params)
    {
        uint32_t hash = rage::atLiteralStringHash (name);
        switch (hash)
            {
            case "SHOW_SHARD_CENTERED_MP_MESSAGE_LARGE"_joaat:
                SetMissionFailInsult (params[1]);
                break;

            case "UPDATE_MESSAGE"_joaat:
                SetMissionFailInsult (params[0], true);
                break;
            }
    }

    /*******************************************************/
    template <auto &CScaleformMgr__AddMethodToBuffer>
    static void *
    RandomizeScaleformMethod (void *p1, int p2, uint64_t p3, uint64_t p4,
                              char *                              name,
                              ScaleformMethodStruct::ParamStruct *params,
                              uint32_t p7, bool p8, bool p9, int p10, int p11)
    {
        m_ScaleformStrings.clear ();

        RandomizeScaleformMethod (name, params);
        return CScaleformMgr__AddMethodToBuffer (p1, p2, p3, p4, name, params,
                                                 p7, p8, p9, p10, p11);
    }

    /*******************************************************/
    static void
    ReadInsultsList ()
    {
        static bool sm_Initialised = false;
        if (std::exchange (sm_Initialised, true))
            return;

        FILE *insultsFile
            = Rainbomizer::Common::GetRainbomizerDataFile ("FailMessages.txt");

        if (!insultsFile)
            return;

        char line[1024] = {0};
        while (fgets (line, 1024, insultsFile))
            {
                line[strcspn (line, "\n")] = 0;
                m_MissionFailInsults.push_back (line);
                m_MissionFailInsults.back () += ' ';
            }

        fclose (insultsFile);
        return;
    }

public:
    /*******************************************************/
    ScaleformRandomizer ()
    {
        if (!ConfigManager ::ReadConfig (
                "ScaleformRandomizer",
                std::make_pair ("FailMessageOdds", &Config ().FailMessageOdds)))
            return;
        ;

        InitialiseAllComponents ();
        REGISTER_HOOK ("89 74 ? ? e8 ? ? ? ? ? 8b cd ? 8b d0 e8", 4,
                       RandomizeScaleformMethod, void *, void *, int, uint64_t,
                       uint64_t, char *, ScaleformMethodStruct::ParamStruct *,
                       uint32_t, bool, bool, int, int);
    }
} _scaleform;
