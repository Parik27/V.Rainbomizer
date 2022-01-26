#include <implot.h>
#include "base.hh"
#include "imgui.h"

#include <deque>

#include <Utils.hh>
#include <CPools.hh>
#include <type_traits>

class PoolsDebugInterface : public DebugInterface
{
    inline static CPool<void> sm_FakeCollidersPool;
    
    std::vector<int>
    GetXValues ()
    {
        std::vector<int> vec;
        for (int i = 0; i < 500; i++)
            vec.push_back(i);

        return vec;
    }

    template <typename T>
    auto
    Dereference (const T &v)
    {
        if constexpr (std::is_pointer_v<T>)
            return Dereference (*v);
        else
            return v;
    }

    template <typename T>
    bool
    CanDereference (T &&v)
    {
        if constexpr (std::is_pointer_v<T>)
            return v && CanDereference (*v);
        else
            return true;
    }

    template <auto &Pool>
    void
    DrawPool (const char *name)
    {
        if (!CanDereference(Pool))
            return;
        
        int PoolUsage = Dereference(Pool).GetCount ();
        int PoolSize  = Dereference(Pool).m_nMaxElements;

        static std::vector<int> x_Values = GetXValues ();
        static std::vector<int> y_Values;

        y_Values.push_back (Dereference(Pool).GetCount ());

        if (y_Values.size () > 500)
            {
                y_Values.erase (y_Values.begin ());
            }

        if (!ImGui::CollapsingHeader (name))
            return;

        ImPlot::SetNextPlotLimits (0, 520, 0, PoolSize + 10, ImGuiCond_Always);
        if (ImPlot::BeginPlot (name))
            {
                ImPlot::SetNextLineStyle (ImVec4 (1.0, 0.0, 0.0, 1.0));
                ImPlot::PlotHLines ("Max", &PoolSize, 1);

                ImPlot::SetNextLineStyle (IMPLOT_AUTO_COL, 5.0);
                ImPlot::PlotLine ("Usage", x_Values.data (), y_Values.data (),
                                  y_Values.size ());

                ImPlot::SetNextLineStyle ();
                ImPlot::EndPlot ();
            }

        ImGui::ProgressBar (PoolUsage / static_cast<float> (PoolSize));
    }

    void
    ProcessPhInstPool ()
    {
        static char **phInstance = GetRelativeReference<char *> (
            "? 8B 0D ? ? ? ? ? 83 64 ? ? 00 ? 0F B7 D1 ? 33 C9 E8", 3, 7);

        static auto [usedCollidersOffset, maxCollidersOffset] = [] {
            auto pattern = hook::pattern (
                "? 63 ? ? ? ? ? 3B ? ? ? ? ? 0F 8D ? ? ? ? ? 8B C8");

            return std::make_tuple (*pattern.get_one ().get<int> (3),
                                    *pattern.get_one ().get<int> (9));
        }();

        if (!*phInstance)
            return;

        sm_FakeCollidersPool.m_nCount = injector::ReadMemory<uint32_t> (
            *phInstance + usedCollidersOffset);
        sm_FakeCollidersPool.m_nMaxElements
            = injector::ReadMemory<uint32_t> (*phInstance + maxCollidersOffset);
    }

    void
    Draw () override
    {
        ProcessPhInstPool ();

        DrawPool<sm_FakeCollidersPool> ("Colliders");
        DrawPool<CPools::g_pVehicleStreamRequestGfxPool> ("Vehicle Stream Request GFX");
        DrawPool<CPools::g_pVehicleStructPool> ("Vehicle Struct");
    }

public:

    const char *
    GetName () override
    {
        return "Pools";
    }

} inline g_PoolsDebugInterface;
