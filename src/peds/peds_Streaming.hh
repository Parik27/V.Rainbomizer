#pragma once

#include <array>
#include <cstdint>
#include <Utils.hh>
#include <CStreaming.hh>
#include <set>

#include <common/common.hh>
#include <common/logger.hh>

#include <rage.hh>
#include <CPopulation.hh>
#include <CAmbientModelSets.hh>

#include <ctime>

#define RANDOMIZE_PED_GROUPS

class PedRandomizer_Streaming
{
    inline static bool sm_Initialised         = false;
    inline static auto sm_LastStreamedPedTime = time (NULL);

    inline static std::vector<uint32_t> sm_NsfwModels;
    inline static std::vector<uint32_t> sm_BlacklistModels;
    inline static bool                  sm_NsfwModelsInitialised      = false;
    inline static bool                  sm_BlacklistModelsInitialised = false;

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
#ifdef RANDOMIZE_PED_GROUPS
        uint32_t ret = CStreaming_GetRandomPedToLoad (str, p2);

        if ((ret & 0xFFFF) != 0xFFFF)
            return ret;
#endif

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
    ReadModelsList (const std::string &file, std::vector<uint32_t> &out,
                    bool &outInitialised)
    {
        if (outInitialised)
            return;
        outInitialised = true;

        FILE *f = Rainbomizer::Common::GetRainbomizerDataFile (file);

        if (!f)
            return;

        char line[256] = {0};
        while (fgets (line, 256, f))
            {
                line[strcspn (line, "\n")] = 0;
                out.push_back (rage::atStringHash (line));
            }
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
        groups->mPedsSet5.for_each ([&peds] (int val) { peds.insert (val); });

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
        if (!includeNsfw
            && (ReadModelsList ("NSFW_Models.txt", sm_NsfwModels,
                                sm_NsfwModelsInitialised),
                true))
            for (auto hash : sm_NsfwModels)
                peds.erase (CStreaming::GetModelIndex (hash));
    }

    /*******************************************************/
    template <auto &CPopGroupList__GetVehGroup>
    static bool
    RandomizePedGroups (CPopGroupList *grps, uint32_t name, uint32_t *out)
    {
        auto &PedHashes = Rainbomizer::Common::GetPedHashes ();

        // Pop Groups
        for (auto &grp : grps->pedGroups)
            {
                for (auto &model : grp.models)
                    model.Name = GetRandomElement (PedHashes);
            }

        // Ambient Model Sets (used for Scenarios)
        for (auto &models : CAmbientModelSetsManager::Get ()
                                ->aSets[AMBIENT_PED_MODEL_SET]
                                .ModelSets)
            {
                for (auto &model : models->Models)
                    model.Name = GetRandomElement (PedHashes);
            }

        return CPopGroupList__GetVehGroup (grps, name, out);
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

        injector::MakeNOP (
            hook::get_pattern (
                "e8 ? ? ? ? ? 8d ? ? ? ? ? ? 8d ? ? ? ? ? ? 3b df 74 ? 0f b7 "
                "47 08 66 39 43 08 74 ? 66 39 73 0a 74 ? ? 8b 0b e8 ? ? ? ? ? "
                "8b d7 ? 8b cb e8 ? ? ? ? 66 41 39 ? ? ? ? ? 77 ? 66 41 39 ? ? "
                "? ? ? 0f 86 ? ? ? ?"),
            5);

#ifdef RANDOMIZE_PED_GROUPS
        REGISTER_HOOK ("ba fc 76 c4 68 e8", 5, RandomizePedGroups, bool,
                       CPopGroupList *, uint32_t, uint32_t *);
#endif

        sm_Initialised = true;
    }

    /*******************************************************/
    static bool
    IsPedBlacklisted (uint32_t idx)
    {
        ReadModelsList ("PedsBlacklist.txt", sm_BlacklistModels,
                        sm_BlacklistModelsInitialised);
        return DoesElementExist (sm_BlacklistModels,
                                 CStreaming::GetModelByIndex (idx)->m_nHash);
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
                for (int i = 0; i < PEDS_TO_LOAD; i++)
                    CStreaming::RequestModel (GetRandomPedIndex (), 0);
                sm_LastStreamedPedTime = time (NULL);
            }
    }

    friend class LoadedPedDebugInterface;
};
