#include "ModUtils/Trampoline.h"
#include "Patterns/Patterns.hh"
#include "imgui.h"
#include <Utils.hh>
#include <common/config.hh>
#include <audEngine.hh>
#include <common/logger.hh>
#include <mutex>
#include <sstream>

#include <debug/base.hh>

#define ENABLE_MUSIC_RANDOMIZER

class MusicRandomizer : public DebugInterface
{
    inline static std::vector<std::pair<uint32_t, uint32_t>> m_SwapPairs{
        {0x8D8B11E3, 0xCC018B79},
        {0xE46E6753, 0x49AAB45},
        //{0xCFC49612, 0x49AAB45}
    };

    inline static std::mutex m_SwapPairsMutex;
    
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
    RandomizeMusic (audMetadataMgr *mgr, uint32_t offset)
    {
        uint32_t hash = mgr->FindObjectHashFromOffset (offset);

        std::lock_guard guard (m_SwapPairsMutex);
        for (auto& [oldH, newH] : m_SwapPairs)
            {
                
                if (hash == oldH) {
                    auto* obj = mgr->FindObjectPtr<rage::Sound> (newH);
                    if (!obj)
                        Rainbomizer::Logger::LogMessage (
                            "Failed to find sound: %x", newH);
                    return obj;
                }
            }
        
        // if (hash == 0xE46E6753 || hash == 0xCFC49612)
        //     return mgr->FindObjectPtr<rage::Sound> (0x49AAB45);
        // if (hash == 0x8D8B11E3)
        //     return mgr->FindObjectPtr<rage::Sound> (0xCC018B79);
            
        rage::Sound *sound = O (mgr, offset);

        // if (sound && ShouldRandomizeSound (sound))
        //     {
        //         std::vector<rage::Sound *> mStreamingSounds;

        //         mgr->ForEach<rage::Sound> ([&] (auto data, uint32_t, uint32_t) {
        //             if (data->Type == sound->Type)
        //                 mStreamingSounds.push_back (data);
        //         });

        //         return GetRandomElement (mStreamingSounds);
        //     }

        return sound;
    }

public:

    void Draw () override
    {
        if (ImGui::Button("Add"))
            {
                std::lock_guard guard (m_SwapPairsMutex);
                m_SwapPairs.push_back({0,0});
            }

        int i = 0;
        ImGui::Columns (3);
        for (auto it = m_SwapPairs.begin (); it != m_SwapPairs.end ();)
            {
                ImGui::PushID (i++);
                ImGui::InputScalar ("##Old", ImGuiDataType_U32, &(*it).first,
                                    NULL, NULL, "%x",
                                    ImGuiInputTextFlags_CharsHexadecimal);
                ImGui::NextColumn();
                ImGui::InputScalar ("##New", ImGuiDataType_U32, &(*it).second,
                                    NULL, NULL, "%x",
                                    ImGuiInputTextFlags_CharsHexadecimal);
                ImGui::NextColumn();

                if (ImGui::Button("X"))
                    {
                        std::lock_guard guard (m_SwapPairsMutex);
                        it = m_SwapPairs.erase (it);
                    }
                else
                    it++;

                ImGui::NextColumn();
                ImGui::PopID ();
            }
        ImGui::Columns ();
    }

    const char* GetName () override
    {
        return "Sounds Randomizer";
    }
    
    /*******************************************************/
    MusicRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("MusicRandomizer"))
            return;

        InitialiseAllComponents ();

        // Hooks:
        // 1. rage::audSoundFactory::GetInstance<rage::audMetadataRef>
        // 2. rage::audSoundFactory::GetChildInstance<rage::audMetadataRef>

        float *speed = Trampoline::MakeTrampoline (GetModuleHandle (nullptr))
                           ->Pointer<float> ();
        injector::MakeRelativeOffset (
            hook::get_pattern ("f3 0f 10 05 ? ? ? ? f3 0f 11 ? ? 83 ? ? 00 ? "
                               "83 ? ? 00 ? 83 ? ? 00 ? 8d ? 6f ? 8d ? cf ",
                               4),
            speed);
        *speed = 30.0f;

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
