#pragma once

#include <common/common.hh>

#include <Utils.hh>

#include <CPopulation.hh>
#include <CAmbientModelSets.hh>
#include <cstdint>

/* Common helper class for both ped randomizer and traffic randomizer to
 * randomize model sets related to vehicles and peds to add more variety */
class ModelsListRandomizer
{
    inline static bool sm_RandomizePeds = false;
    inline static bool sm_RandomizeVehs = false;

    /*******************************************************/
    template <typename T, typename T2>
    static void
    RandomizeGroup (T &array, const T2 &hashes)
    {
        for (auto &grp : array)
            {
                for (auto &model : grp.Models)
                    model.Name = GetRandomElement (hashes);
            }
    }

    /*******************************************************/
    static auto &
    GetAmbientModelSets (AmbientModelSetType id)
    {
        return CAmbientModelSetsManager::Get ()->aSets[id].ModelSets;
    }

    /*******************************************************/
    template <typename T>
    static void
    RandomizeModelSet (AmbientModelSetType type, const T &hashes)
    {
        for (auto &grp : GetAmbientModelSets (type))
            {
                for (auto &model : grp->Models)
                    model.Name = GetRandomElement (hashes);
            }
    }

    /*******************************************************/
    template <auto &CPopGroupList__GetVehGroup>
    static bool
    RandomizePedGroups (CPopGroupList *grps, uint32_t name, uint32_t *out)
    {
        auto &PedHashes = Rainbomizer::Common::GetPedHashes ();
        auto &VehHashes = Rainbomizer::Common::GetVehicleHashes ();

        if (sm_RandomizePeds)
            {
                RandomizeGroup (grps->pedGroups, PedHashes);
                RandomizeModelSet (AMBIENT_PED_MODEL_SET, PedHashes);
            }

        if (sm_RandomizeVehs)
            {
                RandomizeGroup (grps->vehGroups, VehHashes);
                RandomizeModelSet (AMBIENT_VEHICLE_MODEL_SET, VehHashes);
            }

        return CPopGroupList__GetVehGroup (grps, name, out);
    }

    /*******************************************************/
    static void
    InitialiseHooks ()
    {
        static bool s_Initialised = false;
        
        if (std::exchange(s_Initialised, true))
            return;

        REGISTER_HOOK ("ba fc 76 c4 68 e8", 5, RandomizePedGroups, bool,
                       CPopGroupList *, uint32_t, uint32_t *);
    }

public:
    /*******************************************************/
    static void
    Initialise (bool peds, bool vehs)
    {
        sm_RandomizePeds = peds ? true : sm_RandomizePeds;
        sm_RandomizeVehs = peds ? true : sm_RandomizeVehs;

        if (peds || vehs)
            {
                InitialiseHooks ();
            }
    }
};
