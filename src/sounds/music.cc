#include <Utils.hh>
#include <common/config.hh>
#include <audEngine.hh>
#include <common/logger.hh>
#include <sstream>

class MusicRandomizer
{
    /*******************************************************/
    static bool
    ShouldRandomizeSound (rage::Sound *sound)
    {
        switch (sound->Type)
            {
            case audSoundType::audStreamingSound: return true;
            default: return false;
            }
    }

    /*******************************************************/
    template <auto &O>
    static rage::Sound *
    RandomizeMusic (audMetadataMgr *mgr, uint32_t hash)
    {
        rage::Sound *sound = O (mgr, hash);

        if (sound && ShouldRandomizeSound (sound))
            {
                std::vector<rage::Sound *> mStreamingSounds;

                mgr->ForEach<rage::Sound> ([&] (auto data, uint32_t, uint32_t) {
                    if (data->Type == sound->Type)
                        mStreamingSounds.push_back (data);
                });

                return GetRandomElement (mStreamingSounds);
            }

        return sound;
    }

public:
    /*******************************************************/
    MusicRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("MusicRandomizer"))
            return;

        InitialiseAllComponents ();

        // Hooks:
        // 1. rage::audSoundFactory::GetInstance<rage::audMetadataRef>
        // 2. rage::audSoundFactory::GetChildInstance<rage::audMetadataRef>
        REGISTER_HOOK ("8b d9 ? 8b f8 e8 ? ? ? ? ? 8b 54", 5, RandomizeMusic,
                       rage::Sound *, audMetadataMgr *, uint32_t);

        REGISTER_HOOK ("8b f0 e8 ? ? ? ? 8a 54 ? ? 88 54", 2, RandomizeMusic,
                       rage::Sound *, audMetadataMgr *, uint32_t);
    }
}
#ifdef ENABLE_MUSIC_RANDOMIZER
music
#endif
    ;
