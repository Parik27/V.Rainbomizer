#include "Patterns/Patterns.hh"
#include "Utils.hh"
#include "base.hh"
#include "common/common.hh"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <filesystem>
#include <fstream>
#include <misc/worldPS.h>

#include <common/minhook.hh>
#include <common/logger.hh>

#include <CShaderLib.hh>
#include <rage.hh>

#include <regex>
#include <string_view>

void (*ReloadShaders)();

class ShaderDebugInterface : public DebugInterface
{
    inline static bool sm_Enabled = false;
    inline static std::string sm_ShaderFile = "mirrored.fxc";
    inline static int32_t     sm_MatchType  = -1;
    inline static std::string sm_RegexMatch = ".*PS_LensDistortion.*";
    inline static std::string sm_Uncache    = "postfx,postfxms,postfxms0";

    void
    DeleteShader (uint32_t hash)
    {
        auto shader = CShaderLib::LookupShader (hash);
        if (shader)
            delete shader;
    }

    void
    FreeAndReloadShaders ()
    {
        char *str = _strdup (sm_Uncache.c_str ());
        char *pch = strtok (str, ",");

        while (pch)
            {
                DeleteShader (rage::atStringHash(pch));
                pch = strtok(nullptr, ",");
            }
        
        ReloadShaders ();

        free(str);
    }

    void
    Draw () override
    {
        ImGui::Checkbox ("Enabled", &sm_Enabled);

        
        ImGui::InputText ("Shader file", &sm_ShaderFile);
        ImGui::SameLine();
        if (ImGui::BeginCombo("Shader file list", "", ImGuiComboFlags_NoPreview))
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
                                FreeAndReloadShaders ();
                            }

                ImGui::EndCombo();
            }

        ImGui::InputText("Uncache", &sm_Uncache);
        ImGui::InputText ("Replace Shader", &sm_RegexMatch);
        ImGui::InputInt ("Match Type", &sm_MatchType);

        if (ImGui::Button ("Reload"))
            FreeAndReloadShaders ();
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

                        std::vector<uint8_t> data = ReadFile (
                            Rainbomizer::Common::GetRainbomizerFileName (
                                sm_ShaderFile, "shaders/"));

                        return rage__CreateShader (name, data.data (),
                                                   data.size (), type, out);
                    }
            }
        catch (std::regex_error &e)
            {
            }

        return rage__CreateShader (name, data, size, type, out);
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
    }

} g_ShaderDebugInterface;
