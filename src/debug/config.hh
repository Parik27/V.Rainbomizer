#pragma once

#include <string>
#include <variant>
#include <string_view>
#include <map>
#include "base.hh"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

class ConfigDebugInterface : public DebugInterface
{
public:
    using Type = std::variant<int *, std::string *, bool *, double *>;

private:
    inline static std::map<std::string, Type> sm_ConfigOptions;
    static ConfigDebugInterface               sm_Instance;

    void
    Draw () override
    {
        for (auto i : sm_ConfigOptions)
            {
                ImGui::Columns (2);

                ImGui::Text ("%s", i.first.c_str ());
                ImGui::NextColumn ();

                ImGui::PushID (i.first.c_str ());
                std::visit (
                    [i] (auto &&arg) {
                        using T = std::decay_t<decltype (arg)>;

                        if constexpr (std::is_same_v<T, int *>)
                            ImGui::InputInt ("", arg);
                        else if constexpr (std::is_same_v<T, double *>)
                            ImGui::InputDouble ("", arg);
                        else if constexpr (std::is_same_v<T, std::string *>)
                            ImGui::InputText ("", arg);
                        else if constexpr (std::is_same_v<T, bool *>)
                            ImGui::Checkbox ("", arg);
                    },
                    i.second);
                ImGui::PopID ();
                ImGui::NextColumn ();
            }
    }

public:
    template <typename T>
    static void
    AddConfigOption (std::string_view path, T *data)
    {
        sm_ConfigOptions[std::string (path)] = data;
    }

    const char *
    GetName () override
    {
        return "Config";
    }
};

inline ConfigDebugInterface ConfigDebugInterface::sm_Instance{};
