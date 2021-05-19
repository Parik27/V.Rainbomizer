#pragma once

#include <cstdint>
#include <Utils.hh>
#include <CStreaming.hh>
#include <set>

#include <common/common.hh>
#include <rage.hh>

class PedRandomizer_Streaming
{
    inline static bool sm_Initialised         = false;
    inline static auto sm_LastStreamedPedTime = time (NULL);

    inline static std::vector<uint32_t> sm_NsfwModels;
    inline static bool                  sm_NsfwModelsInitialised = false;

    /*******************************************************/
    static uint32_t
    GetRandomPedIndex ()
    {
        auto &PedHashes = Rainbomizer::Common::GetPedHashes ();
        return CStreaming::GetModelIndex (GetRandomElement (PedHashes));
    }

    /*******************************************************/
    template <auto &CStreaming_GetRandomPedToLoad>
    static uint32_t
    RandomizePedToLoad (CStreaming *str, bool p2)
    {
        const int MAX_TRIES = 16;
        for (int i = 0; i < MAX_TRIES; i++)
            {
                uint32_t pedIndex = GetRandomPedIndex ();

                if (CStreaming::HasModelLoaded (pedIndex))
                    continue;

                return pedIndex;
            }

        return 65535; // -1/uint16_t
    }

    /*******************************************************/
    static void
    ReadNsfwModelsList ()
    {
        if (sm_NsfwModelsInitialised)
            return;
        sm_NsfwModelsInitialised = true;

        FILE *f
            = Rainbomizer::Common::GetRainbomizerDataFile ("NSFW_Models.txt");

        if (!f)
            return;

        char line[256] = {0};
        while (fgets (line, 256, f))
            sm_NsfwModels.push_back (rage::atStringHash (line));
    }

    /*******************************************************/
    static void
    PopulateLoadedPedsSet (std::set<uint32_t> &peds, bool includeNsfw)
    {
        auto groups = CStreaming::sm_Instance;

        // Add loaded peds
        groups->mAppropriatePedsSet.for_each (
            [&peds] (int val) { peds.insert (val); });
        groups->mInAppropriatePedsSet.for_each (
            [&peds] (int val) { peds.insert (val); });
        groups->mCopsSet.for_each ([&peds] (int val) { peds.insert (val); });

        // Remove invalid models
        for (auto hash : {"mp_headtargets"_joaat, "slod_large_quadped"_joaat,
                          "slod_human"_joaat, "slod_small_quadped"_joaat})
            peds.erase (CStreaming::GetModelIndex (hash));

        // Add player models
        for (auto hash :
             {"player_one"_joaat, "player_zero"_joaat, "player_two"_joaat})
            {
                auto index = CStreaming::GetModelIndex (hash);
                if (CStreaming::HasModelLoaded (index))
                    peds.insert (index);
            }

        // Remove NSFW models
        if (!includeNsfw && (ReadNsfwModelsList (), true))
            for (auto hash : sm_NsfwModels)
                peds.erase (CStreaming::GetModelIndex (hash));
    }

public:
    /*******************************************************/
    static void
    Initialise ()
    {
        if (sm_Initialised)
            return;

        REGISTER_HOOK (
            "83 ec 20 80 3d ? ? ? ? 00 ? 8b f1 74 ? e8 ? ? ? ? eb ? e8", 22,
            RandomizePedToLoad, uint32_t, CStreaming *, bool);

        sm_Initialised = true;
    }

    /*******************************************************/
    static uint32_t
    GetRandomLoadedPed (bool includeNsfw = false)
    {
        std::set<uint32_t> peds;
        PopulateLoadedPedsSet (peds, includeNsfw);

        if (peds.size () < 1)
            return -1;

        return GetRandomElement (peds);
    }

    /*******************************************************/
    static void
    Process ()
    {
        const int TIMEOUT      = 5;
        const int PEDS_TO_LOAD = 5;

        if (time (NULL) - sm_LastStreamedPedTime > TIMEOUT)
            {
                for (int i = 0; i < 5; i++)
                    CStreaming::RequestModel (GetRandomPedIndex (), 0);
                sm_LastStreamedPedTime = time (NULL);
            }
    }
};
