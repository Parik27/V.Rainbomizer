#include "base.hh"
#include <string>

class ActionDebugInterface : public DebugInterface
{

    void
    Draw () override
    {
        for (auto &i : DebugInterfaceManager::sm_Actions)
            {
                if (ImGui::Button (i.first.c_str ()))
                    i.second = true;
            }

        for (auto &i : DebugInterfaceManager::sm_ActionCbs)
            {
                if (ImGui::Button (i.first.c_str ()))
                    i.second (true);
            }
    }

public:
    const char *
    GetName () override
    {
        return "Actions";
    }
} g_ActionsDebugInterface;
