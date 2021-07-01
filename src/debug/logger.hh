#pragma once

#include "base.hh"
#include "imgui.h"
#include <string_view>

class LoggerDebugInterface : public DebugInterface
{
    ImGuiTextBuffer             buf;
    static LoggerDebugInterface sm_Instance;

    void
    Draw () override
    {
        if (ImGui::Button ("Clear"))
            buf.clear ();

        ImGui::TextUnformatted (buf.begin (), buf.end ());
    }

public:
    static void
    PublishLogMessage (std::string_view msg)
    {
        sm_Instance.buf.append (msg.data (), msg.data () + msg.size ());
        sm_Instance.buf.appendf ("\n");
    }

    const char *
    GetName () override
    {
        return "Log";
    }
};

inline LoggerDebugInterface LoggerDebugInterface::sm_Instance{};
