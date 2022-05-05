#include "CTheScripts.hh"
#include "Patterns/Patterns.hh"
#include "Utils.hh"
#include "base.hh"
#include "common/common.hh"
#include "debug/backend.hh"
#include "fiAssetMgr.hh"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <filesystem>
#include <fstream>

#include <common/minhook.hh>
#include <common/logger.hh>

#include <CShaderLib.hh>
#include <rage.hh>

#include <regex>
#include <stdint.h>
#include <string_view>
#include <vadefs.h>

void (*ReloadShaders)();

class ShaderDebugInterface : public DebugInterface
{
    inline static bool        sm_Enabled    = false;
    inline static std::string sm_ShaderFile = "mirrored.fxc";
    inline static int32_t     sm_MatchType  = -1;
    inline static std::string sm_RegexMatch = ".*PS_LensDistortion.*";
    inline static std::string sm_Uncache    = "postfx,postfxms,postfxms0";
    inline static bool        sm_Reload     = false;

    static void
    DeleteShader (uint32_t hash)
    {
        auto shader = CShaderLib::LookupShader (hash);
        if (shader)
            delete shader;
    }

    static void
    FreeAndReloadShaders ()
    {
        char *str = _strdup (sm_Uncache.c_str ());
        char *pch = strtok (str, ",");

        fiAssetMgr::sm_Instance->PushFolder ("common:/shaders");

        while (pch)
            {
                auto shader
                    = CShaderLib::LookupShader (rage::atStringHash (pch));
                if (shader)
                    assert (shader->LoadFile (pch));

                pch = strtok (nullptr, ",");
            }

        fiAssetMgr::sm_Instance->PopFolder ();
        free (str);
    }

    void
    Update () override
    {
        if (sm_Reload)
            FreeAndReloadShaders ();

        sm_Reload = false;
    }

    void
    Draw () override
    {
        ImGui::Checkbox ("Enabled", &sm_Enabled);

        ImGui::InputText ("Shader file", &sm_ShaderFile);
        ImGui::SameLine ();
        if (ImGui::BeginCombo ("Shader file list", "",
                               ImGuiComboFlags_NoPreview))
            {
                std::string path
                    = Rainbomizer::Common::GetRainbomizerFileName ("",
                                                                   "shaders/");

                for (const auto &entry :
                     std::filesystem::directory_iterator (path))
                    if (entry.path ().extension () == ".fxc")
                        if (ImGui::Selectable (
                                entry.path ().filename ().u8string ().c_str ()))
                            {
                                sm_ShaderFile
                                    = entry.path ().filename ().u8string ();
                                sm_Reload = true;
                            }

                ImGui::EndCombo ();
            }

        ImGui::InputText ("Uncache", &sm_Uncache);
        ImGui::InputText ("Replace Shader", &sm_RegexMatch);
        ImGui::InputInt ("Match Type", &sm_MatchType);

        if (ImGui::Button ("Reload"))
            sm_Reload = true;
    }

public:
    const char *
    GetName () override
    {
        return "Shaders";
    }

    static std::vector<uint8_t>
    ReadFile (const std::string &fileName)
    {
        std::ifstream f (fileName, std::ios::in | std::ios::binary);
        if (!f)
            return {};

        return std::vector<uint8_t> ((std::istreambuf_iterator<char> (f)),
                                     std::istreambuf_iterator<char> ());
    }

    template <auto &rage__CreateShader>
    static void *
    RandomizeShader (char *name, uint8_t *data, uint32_t size, uint32_t type,
                     uint32_t *out)
    {
        try
            {
                std::regex regExp (sm_RegexMatch);
                if (sm_Enabled && (sm_MatchType == -1 || type == sm_MatchType)
                    && std::regex_match (name, regExp))
                    {
                        Rainbomizer::Logger::LogMessage ("Replacing shader %s",
                                                         name);
                        Rainbomizer::Logger::LogMessage (
                            "with %s", sm_ShaderFile.c_str ());

                        std::vector<uint8_t> data = ReadFile (
                            Rainbomizer::Common::GetRainbomizerFileName (
                                sm_ShaderFile, "shaders/"));

                        Rainbomizer::Logger::LogMessage (
                            "Data loaded: %p size=%x", data.data (),
                            data.size ());

                        return rage__CreateShader (name, data.data (),
                                                   data.size (), type, out);
                    }
            }
        catch (std::regex_error &e)
            {
            }

        return rage__CreateShader (name, data, size, type, out);
    }

    static void
    SetShaderState (scrThread::Info *info)
    {
        Rainbomizer::Logger::LogMessage ("SetShaderState(%d, %p)",
                                         info->GetArg<uint64_t> (0),
                                         info->GetArg<const char *> (1));

        sm_Enabled    = info->GetArg<bool> (0);
        sm_ShaderFile = info->GetArg<const char *> (1);

        Rainbomizer::Logger::LogMessage ("%s", sm_ShaderFile.c_str ());
        Rainbomizer::Logger::LogMessage ("%s", info->GetArg<const char *> (1));

        sm_Uncache    = "postfx,postfxms,postfxms0";
        sm_RegexMatch = ".*PS_LensDistortion.*";

        sm_Reload = true;
        
        // BackendWalkieTalkie::sm_Functions.push_back (
        //     [] { FreeAndReloadShaders (); });
    }

    ShaderDebugInterface ()
    {
        REGISTER_MH_HOOK_BRANCH (
            "e8 ? ? ? ? ? 89 ? ? ? ? ? 8b 44 ? ? 89 ? ? ? ? ?", 0,
            RandomizeShader, void *, char *, const uint8_t *, uint32_t, uint32_t,
            void *);

        ReadCall (hook::get_pattern ("89 ? ? ? ? ? e8 ? ? ? ? ? 8b ? e8 ? ? ? "
                                     "? ? 84 ff 74 ? ? 8b ? e8 ? ? ? ?",
                                     27),
                  ReloadShaders);

        NativeManager::AddNative (0x1871102311122343, SetShaderState);
    }

} g_ShaderDebugInterface;
