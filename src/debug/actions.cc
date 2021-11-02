#include "base.hh"
#include <string>

class ActionDebugInterface : public DebugInterface
{

    void
    Draw () override
    {
        for (auto &i : DebugInterfaceManager::GetActions ())
            {
                if (ImGui::Button (i.first.c_str ()))
                    i.second = true;
            }

        for (auto &i : DebugInterfaceManager::GetActionCbs ())
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
