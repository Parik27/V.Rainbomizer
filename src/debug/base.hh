#include <functional>
#include <memory>
#include <vector>
#include <imgui.h>
#include <cstdint>
#include <scrThread.hh>
#include <map>

class DebugInterface
{
    bool m_bOpen = false;

    virtual void Draw () = 0;
    virtual void Update (){};

    friend class DebugInterfaceManager;

    virtual void
    Process (uint64_t *stack, uint64_t *globals, scrProgram *program,
             scrThreadContext *ctx)
    {
    }

public:
    virtual const char *GetName () = 0;

    DebugInterface ();

    virtual ~DebugInterface () {}
};

class DebugInterfaceManager
{
    using ActionCallback = std::function<void (bool)>;

    static auto &
    GetDebugInterfaces ()
    {
        static std::vector<DebugInterface *> m_DebugInterfaces;
        return m_DebugInterfaces;
    }

    static auto &
    GetActions ()
    {
        static std::map<std::string, bool, std::less<>> sm_Actions;
        return sm_Actions;
    }

    static auto &
    GetActionCbs ()
    {
        static std::map<std::string, ActionCallback, std::less<>> sm_ActionCbs;
        return sm_ActionCbs;
    }

    inline static bool m_BlockControls;
    inline static bool m_EnableFloating = false;

public:
    static void
    DrawDashboard ()
    {
        if (ImGui::BeginMainMenuBar ())
            {
                if (ImGui::BeginMenu ("Interfaces"))
                    {
                        for (auto i : GetDebugInterfaces ())
                            if (ImGui::MenuItem (i->GetName ()))
                                i->m_bOpen = true;

                        ImGui::EndMenu ();
                    }

                if (ImGui::BeginMenu ("Actions"))
                    {
                        for (auto &i : GetActions ())
                            if (ImGui::MenuItem (i.first.c_str ()))
                                i.second = true;

                        ImGui::Separator ();

                        for (auto &i : GetActionCbs ())
                            if (ImGui::MenuItem (i.first.c_str ()))
                                i.second (true);

                        ImGui::EndMenu ();
                    }

                if (ImGui::BeginMenu ("Options"))
                    {
                        ImGui::Checkbox ("Floating", &m_EnableFloating);
                        ImGui::EndMenu ();
                    }

                ImGui::EndMainMenuBar ();
            }
    }

    static void
    DrawAll ()
    {
        DrawDashboard ();

        for (auto i : GetDebugInterfaces ())
            {
                if (!i->m_bOpen)
                    continue;

                if (ImGui::Begin (i->GetName (), &i->m_bOpen))
                    i->Draw ();
                ImGui::End ();
            }
    }

    static void
    UpdateAll ()
    {
        for (auto i : GetDebugInterfaces ())
            {
                i->Update ();
            }
    }

    static bool
    GetAction (std::string_view name)
    {
        auto [obj, set] = GetActions ().emplace (name, false);
        return std::exchange (obj->second, false);
    }

    static void
    AddAction (std::string_view name, ActionCallback cb)
    {
        GetActionCbs ().emplace (name, cb);
    }

    static void
    RegisterInterface (DebugInterface &intf)
    {
        GetDebugInterfaces ().push_back (&intf);
    }

    static void
    SetControlsBlocked (bool blocked)
    {
        m_BlockControls = blocked;
    }

    static void RunThreadHook (uint64_t *stack, uint64_t *globals,
                               scrProgram *program, scrThreadContext *ctx);

    DebugInterfaceManager ();

    friend class ActionDebugInterface;
};
