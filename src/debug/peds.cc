#include <cstdio>
#include "Utils.hh"
#include "base.hh"

#include <array>

#include <common/common.hh>
#include <stdio.h>

#include "imgui.h"
#include "implot.h"

#include "peds/peds_Streaming.hh"
#include "vehicles/vehicle_common.hh"
#include "rage.hh"

#include <iterator>

class LoadedEntityDebugInterface : public DebugInterface
{
    std::map<uint32_t, std::string> m_Hashes;

    void
    DrawSet (const std::set<uint32_t> &loadedSet, const char *name)
    {
        if (!ImGui::CollapsingHeader (name))
            return;

        ImGui::Columns (3);
        for (auto i : loadedSet)
            {
                uint32_t hash = CStreaming::GetModelHash (i);
                
                if (auto *name = LookupMap (m_Hashes, hash))
                    ImGui::Text ("%s", name->c_str ());
                else
                    ImGui::Text ("%08x", hash);

                ImGui::NextColumn ();
            }
    }

    void
    Draw () override
    {
        std::set<uint32_t> pedsSet;
        std::set<uint32_t> vehsSet;
        PedRandomizer_Streaming::PopulateLoadedPedsSet (pedsSet, true);
        vehsSet = VehicleRandomizerHelper::GetLoadedVehSet ();

        DrawSet (pedsSet, "Peds");
        DrawSet (vehsSet, "Vehicles");
    }

public:
    const char *
    GetName () override
    {
        return "Loaded Entities";
    }

    LoadedEntityDebugInterface ()
    {
        FILE *hashes
            = Rainbomizer::Common::GetRainbomizerDataFile ("Hashes.txt");

        if (!hashes)
            return;

        char line[256] = {0};
        while (fgets (line, 256, hashes))
            {
                line[strcspn (line, "\n")]          = 0;
                m_Hashes[rage::atStringHash (line)] = line;
            }

        fclose (hashes);
    }

} g_LoadedEntityDebugInterface;
