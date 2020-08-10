#include "Utils.hh"
#include "UtilsHooking.hh"
#include <unordered_map>
#include <vector>
#include <rage.hh>
#include <CText.hh>
#include <common/logger.hh>
#include <CStreaming.hh>
#include <common/common.hh>
#include <algorithm>
#include <audEngine.hh>
#include <thread>

class audScriptAudioEntity;
class audSpeechSound;

// audSpeechManager::FindRandomValidVariationForVoiceAndContext
uint32_t (*FindRandomValidVariationForVoiceAndContext_93837) (void *, uint32_t,
                                                              uint32_t,
                                                              uint32_t *,
                                                              bool *, uint8_t);
bool (*audSpeechSound_InitSpeech4316c) (audSpeechSound *, uint32_t, uint32_t,
                                        uint32_t);

char *(*CText__GetText6f8e13) (CText *, const char *);

struct SoundPair
{
    uint32_t    bankHash;
    uint32_t    soundHash;
    std::string subtitle;
};

class VoiceLineRandomizer
{
    static std::unordered_map<uint32_t, const SoundPair *> mAudioPairs;
    static std::unordered_map<uint32_t, std::string>       mSubtitles;
    static std::vector<SoundPair>                          mSounds;

    /*******************************************************/
    static bool
    ShouldRandomizeVoiceLine (uint32_t hash)
    {
        return true;
    }

    /*******************************************************/
    static bool
    RandomizeConversationLine (audScriptAudioEntity *, uint32_t, uint32_t,
                               char *sound, const char *&subtitle, uint32_t,
                               uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
                               uint32_t, uint8_t, uint8_t, uint8_t)
    {
        if (mSounds.size () > 0
            && ShouldRandomizeVoiceLine (rage::atStringHash (subtitle)))
            {
                auto &newSound = GetRandomElement (mSounds);
                if (strstr (sound, "SFX_") == sound)
                    return true;

                subtitle = newSound.subtitle.c_str ();
                mAudioPairs[rage::atPartialStringHash (sound)] = &newSound;
            }

        return true;
    }

    /*******************************************************/
    inline static void
    CorrectVoiceAndContextHashes (uint32_t &voice, uint32_t &context)
    {
        if (mAudioPairs.count (context))
            {
                const auto &pair = *mAudioPairs[context];
                voice            = pair.bankHash;
                context          = pair.soundHash;
            }
    }

    /*******************************************************/
    static uint32_t
    CorrectFindRandomVariation (void *p1, uint32_t voice, uint32_t context,
                                uint32_t *p4, bool *p5, uint8_t p6)
    {
        CorrectVoiceAndContextHashes (voice, context);
        return FindRandomValidVariationForVoiceAndContext_93837 (p1, voice,
                                                                 context, p4,
                                                                 p5, p6);
    }

    /*******************************************************/
    static bool
    CorrectInitSpeech (audSpeechSound *sound, uint32_t voiceHash,
                       uint32_t soundPartial, uint32_t variant)
    {
        CorrectVoiceAndContextHashes (voiceHash, soundPartial);
        return audSpeechSound_InitSpeech4316c (sound, voiceHash, soundPartial,
                                               variant);
    }

    /*******************************************************/
    static bool
    CorrectDoesTextLabelExist (CText *p1, const char *label)
    {
        uint32_t labelHash = rage::atStringHash (label);

        if (CText::TheText->GetText (labelHash))
            return true;

        return mSubtitles.count (labelHash);
    }

    /*******************************************************/
    static const char *
    CorrectSubtitle (CText *p1, const char *label)
    {
        // It's an offset to CText::TheText->LabelFoundAt and stuff
        static uint32_t &CText__LabelFoundAt = *hook::get_pattern<uint32_t> (
            "8b 35 ? ? ? ? ? 38 ? ? ? ? ? 0f 84", 2);

        uint32_t labelHash = rage::atStringHash (label);

        if (mSubtitles.count (labelHash))
            {
                CText__LabelFoundAt = 6;
                return mSubtitles[labelHash].c_str ();
            }

        return CText__GetText6f8e13 (p1, label);
    }

    /*******************************************************/
    static void
    InitialiseHooks ()
    {
        // Add conversation line hook. Used for editing the subtitle.
        ReplaceJmpHook__fastcall<0xe4440, void, audScriptAudioEntity *,
                                 uint32_t, uint32_t, char *, const char *,
                                 uint32_t, uint8_t, uint8_t, uint8_t, uint8_t,
                                 uint8_t, uint32_t, uint8_t, uint8_t, uint8_t> (
            hook::get_pattern ("? 8b c4 ? 89 ? ? 89 ? ? ? 89 ? ? 55 53 56 57 ? "
                               "54 ? 55 ? 56 ? 57 ? 8d ? f8 fc ff ff"),
            RandomizeConversationLine)
            .Activate ();

        // To correct the random variation the audio engine gets for the sounds.
        // <true> = JMP instruction
        RegisterHook<true> ("8b ce 8b d3 ? 8b c0 ? 8b cf ? 8b 5c ? ? ? 8b 74 ? "
                            "? ? 83 c4 30 5f e9",
                            25,
                            FindRandomValidVariationForVoiceAndContext_93837,
                            CorrectFindRandomVariation);

        // To correct the actual sound name it uses for initialising the speech
        // sound.
        RegisterHook ("8b cd 8b d0 ? 89 7c ? ? ? 89 74 ? ? e8 ? ? ? ? ? 8b 5c "
                      "? ? ? 8b 6c",
                      14, audSpeechSound_InitSpeech4316c, CorrectInitSpeech);

        // This hook is used to ensure that the game uses the correct subtitle
        // for a multiple-variation voice line. Game checks if "%s_02d" exists,
        // if not use the normal subtitle.
        RegisterHook ("8d ? ? 70 ? 8b ? e8 ? ? ? ? ? 8b cf 84 c0 74", 7,
                      CorrectDoesTextLabelExist);

        RegisterHook ("e8 ? ? ? ? ? 8b e8 ? 85 c0 74 ? ? 84 e4 75 ? ?", 0,
                      CText__GetText6f8e13, CorrectSubtitle);
    }

    /*******************************************************/
    static bool
    ReadSubtitlesForVoiceLine (const char *gxt, const char *subtitleLabel)
    {
        const uint32_t SLOT = 18; // Slot to load the gxt in

        uint32_t subtitle_pH = rage::atPartialStringHash (subtitleLabel);
        if (!CText::TheText->HasThisAdditionalTextLoaded (gxt, SLOT))
            CText::TheText->RequestAdditionalText (SLOT, gxt, true, 2);

        if (!CText::TheText->HasThisAdditionalTextLoaded (gxt, SLOT))
            return false;

        auto AddSubtitle = [&] (const char *suffix) -> bool {
            uint32_t    hash     = rage::atStringHash (suffix, subtitle_pH);
            const char *subtitle = CText::TheText->GetText (hash);

            if (subtitle)
                mSubtitles[hash] = subtitle;

            return subtitle;
        };

        AddSubtitle ("");
        for (int i = 1; i <= 50; i++)
            {
                char buffer[4] = {0};
                sprintf (buffer, "_%02d", i);

                if (!AddSubtitle (buffer))
                    break;
            }

        return true;
    }

    /*******************************************************/
    static void
    InitialiseSoundsList ()
    {
        if (mSounds.size ())
            return;

        FILE *soundsFile
            = Rainbomizer::Common::GetRainbomizerDataFile ("VoiceLines.txt");

        if (!soundsFile)
            return;

        char line[256] = {0};
        while (fgets (line, 256, soundsFile))
            {
                char context[32]  = {0}; // Most are 10 characters long
                char gxt[16]      = {0}; // limit enforced by dialogue_handler
                char voice[64]    = {0};
                char subtitle[64] = {0};

                sscanf (line, "%s %s %s %s", context, gxt, voice, subtitle);

                if (ReadSubtitlesForVoiceLine (gxt, subtitle))
                    mSounds.push_back ({rage::atStringHash (voice),
                                        rage::atPartialStringHash (context),
                                        subtitle});
            }
        Rainbomizer::Logger::LogMessage ("Initialised %d voice lines",
                                         mSubtitles.size ());
    }

public:
    /*******************************************************/
    VoiceLineRandomizer ()
    {
        InitialiseAllComponents ();
        InitialiseHooks ();

        Rainbomizer::Common::AddInitCallback ([] (bool session) {
            if (session)
                InitialiseSoundsList ();
        });
    }
} voices;

std::unordered_map<uint32_t, const SoundPair *>
                                          VoiceLineRandomizer::mAudioPairs;
std::vector<SoundPair>                    VoiceLineRandomizer::mSounds;
std::unordered_map<uint32_t, std::string> VoiceLineRandomizer::mSubtitles;
