#include "CTheScripts.hh"
#include "common/logger.hh"
#include "rage.hh"
#include "scrThread.hh"
#include <Utils.hh>
#include <array>
#include <cstdint>
#include <CScaleformMgr.hh>
#include <string>

using namespace std::string_literals;

/* 123robot wrote these so if you don't like them shout at him thanks Kappa */
const std::array<std::string, 88> INSULTS = {
    "You fool!",
    "You idiot! ",
    "Too tough for you, loser?",
    "Wow, you bloody useless moron, how did you manage that?",
    "You absolute disaster of a human being.",
    "That was the absolute worst performance I've ever seen.",
    "Is this your first time playing this game? Looks like it.",
    "You're a big fat stupid potatohead.",
    "You let down yourself like you let down your family name.",
    "I feel like I lost IQ points just from witnessing that.",
    "Were you dropped on your head as a child?",
    "I hate to tell you this, but that sucked.",
    "That was awful.",
    "You're rubbish.",
    "Man, you really suck at this game.",
    "Is this your first time playing a video game?",
    "Your existence was a mistake.",
    "You big dumb doodoo head.",
    "Only someone very special could fail like that.",
    "Do you understand how to operate these controls? Do you need an adult?",
    "Pro Tip: Complete the mission objectives properly to not lose.",
    "You play like you have one arm and half a brain.",
    "All you had to do was pass the damn mission.",
    "Aw shit, here you go again.",
    "This explains why no one loves you.",
    "What a complete moron you are.",
    "How could anyone do something so stupid?",
    "I really thought I'd seen the worst GTA V players before I saw you.",
    "You must be the dumbest person on this planet. Really.",
    "Maybe you should see a therapist, you need serious help.",
    "That's a good candidate for a Top GTA V Fails video.",
    "You've brought shame to your entire family and everyone who knows you.",
    "You are so so stupid!",
    "Oh, little baby gonna cry, can't handle this? Boohoo!",
    "I'm afraid this game might be a bit too much for you to handle.",
    "Oh, is it too hard for you? Too hard? TOO BAD!",
    "Wow, that was some really shoddy gameplay.",
    "You're a huge disappointment.",
    "You really are colosally useless.",
    "This is why everyone hates you.",
    "I can see why you got dumped for the tennis coach.",
    "You are really dumb.",
    "I've seen dogs that could do better than that.",
    "You're an utter buffoon.",
    "What a clown you are.",
    "What kind of player are YOU anyway...?!",
    "You are such a boneheaded dolt.",
    "Your gameplay is as dreadful as your appearance.",
    "You're a complete dimwit.",
    "You are a goddamned worthless twit.",
    "You talk a big game but you certainly don't bring it.",
    "You're the type of person who's the first to die in a horror movie.",
    "You don't have very much initiative. Or much intelligence.",
    "The results are back from the doctor - you're idiot positive.",
    "Wow, you're such a loser baby.",
    "You scared, baby boy? Go cry to your mommy.",
    "You've got some little bitty chiquita cojones.",
    "Man, you're a buster.",
    "Always letting the homies down, eh?",
    "You're making us all really sad.",
    "Are you normally this much of a disappointment?",
    "You should give up.",
    "Absolute garbage.",
    "Absolutely terrible performance.",
    "This is looking really really bad for you.",
    "That was a complete and utter disaster.",
    "Boohoo!"

    "Don't show this to your friends if you have any. They'd think much "
    "less of you. If they did think anything of you.",

    "I would say I'm disappointed in you, but that would imply I had any "
    "expectations of you to begin with.",

    "Alright child, get off the computer and let an actual grown-up play, "
    "this is embarrassing.",

    "If this is too much for you, there's always the option to give up and "
    "quit.",

    "I can't say I've ever seen someone fail like that before, I'll be "
    "honest.",

    "Perhaps you should consider using cheat codes - maybe that'll help "
    "make up for your exceedingly poor gameplay.",

    "Science indicates that poor video game performance correlates with "
    "poor childhood and lack of friends.",

    "You must have done that on purpose, right? How else could you have "
    "failed so badly?",

    "You're a total failure in life and this proves it.",
    "You should be ashamed of yourself.",

    "Imagine the dumbest, most inept person you know - now realise that "
    "you're even worse than them.",

    "Maybe you should get someone to help you since you're clearly not "
    "capable.",

    "At this rate, you'll be lucky to finish this game before human life "
    "ceases to exist.",

    "There were so many things wrong with what you just did that I'm not "
    "even sure where to begin.",

    "People like you are such a lost cause that you aren't even worth "
    "trying to help.",

    "You're an argument against the education system - clearly it hasn't "
    "helped you.",

    "If GTA V is too complicated for you, maybe you should go back and "
    "start with Pong.",

    "You're a stupid failure, and by the way you're ugly too. And so's your "
    "dad.",

    "Check the dictionary for moron - you should find a picture of "
    "yourself.",

    "Rockstar would like you to know that after seeing that, they have "
    "revoked your permission to continue playing this video game. Please "
    "close it now and never touch it again, for everyone's sake.",

    "I believe people these days would call that a rather small brained "
    "move.",
};

class ScaleformRandomizer
{
    /*******************************************************/
    inline static std::vector<std::string> m_ScaleformStrings;

    /*******************************************************/
    static void
    SetMissionFailInsult (ScaleformMethodStruct::ParamStruct &parameter,
                          bool useLastInsult = false)
    {
        static const int INSULT_ODDS = 10;

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

                // This is done to include it in the fail text's <font> tag. A
                // fallback in case it doesn't exist.

                if (m_LastInsult.find ("'>") != m_LastInsult.npos)
                    m_LastInsult.insert (m_LastInsult.find ("'>") + 2,
                                         GetRandomElement (INSULTS));
                else
                    m_LastInsult
                        = GetRandomElement (INSULTS) + m_LastInsult;

                // Reset it if odds not right.
                if (RandomInt (100) >= INSULT_ODDS)
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
                SetMissionFailInsult(params[1]);
                break;

            case "UPDATE_MESSAGE"_joaat:
                SetMissionFailInsult(params[0], true);
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

public:

    /*******************************************************/
    ScaleformRandomizer ()
    {
        InitialiseAllComponents ();
        REGISTER_HOOK ("89 74 ? ? e8 ? ? ? ? ? 8b cd ? 8b d0 e8", 4,
                       RandomizeScaleformMethod, void *, void *, int, uint64_t,
                       uint64_t, char *, ScaleformMethodStruct::ParamStruct *,
                       uint32_t, bool, bool, int, int);
    }
} _scaleform;
