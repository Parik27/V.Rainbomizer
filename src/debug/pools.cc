#include <implot.h>
#include "base.hh"
#include "common/minhook.hh"
#include "imgui.h"

#include "common/logger.hh"

#include <deque>

#include <Utils.hh>
#include <CPools.hh>
#include <type_traits>

static std::map<uint32_t, std::string> s_PoolsNameMap = {
    {0x0AB968F6, "streamped req data"},
    {0x6E63BFF3, "cpatrolnode"},
    {0x85FB462D, "pedprop req data"},
    {0x54753C86, "cvehiclestreamrequestgfx"},
    {0xB4E04AA4, "cdoorextension"},
    {0x1182232C, "cgamescripthandler"},
    {0xC2B5F089, "cvehiclestreamrendergfx"},
    {0x2D370C34, "streamped render data"},
    {0xD29EB05B, "fwcontainerlod"},
    {0x539C8EB8, "pedprop render data"},
    {0x5E047C3E, "vehicleglasscomponententity"},
    {0x652A20AF, "targetting"},
    {0x4E2ADF9B, "itemsetbuffer"},
    {0x2FEAD7A6, "fraginstnmgta"},
    {0x5047B2C4, "cmoveped"},
    {0x8DA12117, "peds"},
    {0x394AC584, "pedintelligence"},
    {0xAC9F5436, "cmovevehicle"},
    {0xBF69BB29, "fwanimdirectorcomponentragdoll"},
    {0x210E10BE, "fwanimdirectorcomponentfacialrig"},
    {0x16A315A2, "fwanimdirectorcomponentsyncedscene"},
    {0xBAE33CCF, "cweaponcomponentinfo"},
    {0x94E57350, "fwdynamicentitycomponent"},
    {0x3CC59946, "fwanimdirectorcomponentexpressions"},
    {0x7311A8D7, "fwscriptguid"},
    {0xE7509171, "quadtreenodes"},
    {0x3A2BC128, "cweapon"},
    {0xCA5ED810, "fwanimdirector"},
    {0xC79C63D2, "fwanimdirectorcomponentmove"},
    {0x36776CCA, "fwanimdirectorcomponentcreature"},
    {0x698208CF, "tasksequenceinfo"},
    {0x975AC445, "wheels"},
    {0x6851786C, "fraginstgta"},
    {0xEB9564E1, "phinstgta"},
    {0x41663795, "maxloadrequestedinfo"},
    {0x2EB2EE6E, "navmeshes"},
};

class PoolsDebugInterface : public DebugInterface
{
    constexpr static int GRAPH_WIDTH = 500;

    class PoolData
    {
    public:
        std::string Name;

        std::function<std::pair<int, int> ()> GetFunction;
        std::vector<float>                    GraphValues;
        bool                                  IsCritical = false;

        bool AlwaysDisplay   = false;
        bool DisplayCritical = true;
        bool DisplaySeparate = false;

        PoolData (const PoolData &other) = delete;
        PoolData (PoolData &&other)      = default;

        PoolData (std::string name, std::function<std::pair<int, int> ()> func,
                  bool always = false, bool critical = true,
                  bool separate = false)
            : Name (name), GetFunction (func), AlwaysDisplay (always),
              DisplayCritical (critical), DisplaySeparate (separate)
        {
        }

        template <typename T>
        PoolData (std::string name, CPool<T> *pool, bool always = false,
                  bool critical = true, bool separate = false)
            : GetFunction ([pool] () -> std::pair<int, int> {
                  if (!pool)
                      return {0, 0};
                  return {pool->GetCount (), pool->m_nMaxElements};
              }),
              Name (name), AlwaysDisplay (always), DisplayCritical (critical),
              DisplaySeparate (separate)
        {
        }

        template <typename T>
        PoolData (std::string name, CPool<T> **pool, bool always = false,
                  bool critical = true, bool separate = false)
            : GetFunction ([pool] () -> std::pair<int, int> {
                  if (!pool || !(*pool))
                      return {0, 0};
                  return {(*pool)->GetCount (), (*pool)->m_nMaxElements};
              }),
              Name (name), AlwaysDisplay (always), DisplayCritical (critical),
              DisplaySeparate (separate)
        {
        }

        void
        Update ()
        {
            auto [used, max] = GetFunction ();

            if (max == 0)
                return;

            GraphValues.push_back (used / static_cast<float> (max));

            if (GraphValues.size () >= GRAPH_WIDTH)
                GraphValues.erase (GraphValues.begin ());

            IsCritical = used > 10 && (GraphValues.back () > 0.5f);
        }

        bool
        ShouldDisplay ()
        {
            return AlwaysDisplay || (DisplayCritical && IsCritical);
        }
    };

    inline static std::vector<PoolData> sm_Pools;
    inline static uint32_t              sm_LastPool;

    void
    UpdatePools ()
    {
        for (auto &i : sm_Pools)
            i.Update ();
    }

    void
    DrawMasterGraph ()
    {
        if (!ImGui::CollapsingHeader ("Master Graph"))
            return;

        ImPlot::SetNextPlotLimits (0, 520, 0, 1.2f, ImGuiCond_Always);
        if (!ImPlot::BeginPlot ("Master Graph"))
            return;

        static float One = 1.0f;

        ImPlot::SetNextLineStyle (ImVec4 (1.0, 0.0, 0.0, 1.0), 5.0f);
        ImPlot::PlotHLines ("Max", &One, 1);

        ImPlot::SetNextLineStyle (IMPLOT_AUTO_COL);

        static std::vector<float> x_Values = GetXValues ();
        for (auto &i : sm_Pools)
            {
                if (i.ShouldDisplay ())
                    {
                        ImPlot::SetNextLineStyle (IMPLOT_AUTO_COL, 2.0f);
                        ImPlot::PlotLine (i.Name.c_str (), x_Values.data (),
                                          i.GraphValues.data (),
                                          i.GraphValues.size ());
                    }
            }

        ImPlot::EndPlot ();
    }

    void
    DrawPoolGraph (PoolData &data)
    {
        if (!ImGui::CollapsingHeader (data.Name.c_str ()))
            return;

        ImPlot::SetNextPlotLimits (0, 520, 0, 1.0f, ImGuiCond_Always);

        if (!ImPlot::BeginPlot (data.Name.c_str ()))
            return;

        static float              One      = 1.0f;
        static std::vector<float> x_Values = GetXValues ();

        ImPlot::SetNextLineStyle (ImVec4 (1.0, 0.0, 0.0, 1.0));
        ImPlot::PlotHLines ("Max", &One, 1);

        ImPlot::SetNextLineStyle (IMPLOT_AUTO_COL, 5.0f);

        ImPlot::PlotLine ("Usage", x_Values.data (), data.GraphValues.data (),
                          data.GraphValues.size ());

        ImPlot::SetNextLineStyle ();

        ImPlot::EndPlot ();
    }

    void
    DrawPools ()
    {
        UpdatePools ();
        DrawMasterGraph ();
        for (auto &i : sm_Pools)
            {
                if (i.DisplaySeparate)
                    DrawPoolGraph (i);
            }
    }

    std::vector<float>
    GetXValues ()
    {
        std::vector<float> vec;
        for (int i = 0; i < 500; i++)
            vec.push_back (i);

        return vec;
    }

    static std::pair<int, int>
    GetCollidersUsage ()
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
            return {0, 0};

        return {
            injector::ReadMemory<uint32_t> (*phInstance + usedCollidersOffset),
            injector::ReadMemory<uint32_t> (*phInstance + maxCollidersOffset)};
    }

    void
    Draw () override
    {
        DrawPools ();
    }

    template <auto &fwConfigManager__GetSizeOfPool>
    static uint32_t
    GetNextPoolName (void *p1, uint32_t hash, uint32_t p3)
    {
        sm_LastPool = hash;
        return fwConfigManager__GetSizeOfPool (p1, hash, p3);
    }

    template <auto &CPool__Allocate>
    static void
    RegisterPool (CPool<void> *pool)
    {
        if (sm_Pools.size () == 0)
            {
                sm_Pools.emplace_back ("colliders", GetCollidersUsage, true,
                                       true, true);

                sm_Pools.emplace_back ("vehicle struct",
                                       CPools::g_pVehicleStructPool, true, true,
                                       true);
            }

        if (pool->m_nMaxElements > 10 && s_PoolsNameMap.count (sm_LastPool))
            sm_Pools.emplace_back (s_PoolsNameMap[sm_LastPool], pool, true,
                                   true, true);

        return CPool__Allocate (pool);
    }

public:

    const char *
    GetName () override
    {
        return "Pools";
    }

    PoolsDebugInterface ()
    {
        REGISTER_MH_HOOK_BRANCH ("ba 67 ee cc f8 ? b8 10 00 00 00 e8 ? ? ? ?",
                                 11, GetNextPoolName, uint32_t, void *,
                                 uint32_t, uint32_t);

        REGISTER_MH_HOOK ("? 53 ? 83 ec 20 ? 83 39 00 ? 8b d9 75 ? 8b 41 14 0f "
                          "af 41 10 ? 63 c8 ",
                          0, RegisterPool, void, CPool<void> *);
    }

} inline g_PoolsDebugInterface;
