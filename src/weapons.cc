#include "Utils.hh"
#include "common.hh"
#include "logger.hh"
#include "CEntity.hh"
#include "CPed.hh"
#include "UtilsHooking.hh"
#include "rage.hh"
#include "CItemInfo.hh"
#include <random>
#include <algorithm>
#include <ctime>
#include <scrThread.hh>
#include <array>

class WeaponRandomizer
{
    static std::vector<uint32_t> mValidWeapons;
    static std::discrete_distribution<int> mDistribution;

    /*******************************************************/
    static void
    InitialiseWeaponWeights ()
    {
        FILE *file
            = Rainbomizer::Common::GetRainbomizerDataFile ("WeaponWeights.txt");
        std::unordered_map<uint32_t, double> probabilities;

        char line[256] = {0};

        double sum_probability     = 0.0;
        int    total_probabilities = 0;

        if (file)
            {
                while (fgets (line, 256, file))
                    {
                        if (line[0] == '#' || strlen (line) < 5)
                            continue;

                        char   model[128]  = {0};
                        double probability = 0.0;
                        if (sscanf (line, "%s = %lf", model, &probability) == 2)
                            {
                                probabilities[rage::atStringHashLowercase (
                                    model)]
                                    = probability;
                                sum_probability += probability;
                                total_probabilities++;
                            }
                    }
            }

        double mean_probability = 1;

        std::vector<double> weights;
        for (auto i : mValidWeapons)
            {
                if (probabilities.count (i))
                    weights.push_back (probabilities[i]);
                else
                    weights.push_back (mean_probability);
            }

        mDistribution
            = std::discrete_distribution<int>{weights.begin (), weights.end ()};
    }

    /*******************************************************/
    static void
    InitialiseWeaponsArray ()
    {
        if (mValidWeapons.size())
            return;

        mValidWeapons.clear();

        // These models are exempt from randomization
        const std::vector<uint32_t> mExceptions = {};

        for (int i = 0; i < CWeaponInfoManager::sm_Instance->m_nInfosCount; i++)
            {
                auto info = CWeaponInfoManager::GetInfoFromIndex (i);
                uint32_t  modelHash = info->GetModelHash ();

                static_assert("cweaponinfo"_joaat == 0x861905b4);
                
                if (modelHash
                    && std::find (mExceptions.begin (), mExceptions.end (),
                                  info->Name)
                           == mExceptions.end () &&
                    info->GetClassId() == "cweaponinfo"_joaat)
                    mValidWeapons.push_back (info->Name);
            }

        InitialiseWeaponWeights ();
        Rainbomizer::Logger::LogMessage ("Initialised %d valid weapons",
                                         mValidWeapons.size ());
    }
    
    /*******************************************************/
    static void
    PrintWeaponList ()
    {
        for (int i = 0; i < CWeaponInfoManager::sm_Instance->m_nInfosCount; i++)
            {
                auto info = CWeaponInfoManager::sm_Instance->m_paInfos[i];

                Rainbomizer::Logger::LogMessage (
                    "classId => %x, modelId = %x, name = %x",
                    info->GetClassId (), info->GetModelHash (), info->Name);
            }
    }

    /*******************************************************/
    static bool
    DoesWeaponMatchPattern (uint32_t weapon)
    {
        // TODO: (If required) Weapon Patterns
        return false;
    }

    /*******************************************************/
    static int
    GetNewWeaponForWeapon (uint32_t weapon, bool player, std::mt19937 &engine)
    {
        if (scrThread::GetActiveThread () && player
            && DoesWeaponMatchPattern (weapon))
            return weapon;
        
        return mValidWeapons[mDistribution (engine)];
    }

    /*******************************************************/
    static uint32_t
    GetNewWeaponForWeapon (uint32_t weapon, CPedInventory* weapons)
    {
        thread_local static std::mt19937 engine{(unsigned int) time (NULL)};

        // CPed Constructor AddWeapon?
        if (!weapons->m_pPed->m_pModelInfo || weapon == 0)
            return weapon;
        
        bool isPlayer = ((CPedModelInfo *) weapons->m_pPed->m_pModelInfo)
                            ->m_bIsPlayerType;

        // Don't randomize the weapon if it's the player's weapon and it wasn't
        // a thread that gave it to the player
        if (!scrThread::GetActiveThread () && isPlayer)
            return weapon;

        if (std::find (mValidWeapons.begin (), mValidWeapons.end (), weapon)
            != mValidWeapons.end ())
            return GetNewWeaponForWeapon (weapon, isPlayer, engine);

        return weapon;
    }
    
    /*******************************************************/
    static void
    RandomizeWeapon (CPedInventory *weap, uint32_t &hash, uint32_t ammo)
    {
        static bool listPrinted = false;
        if (!std::exchange(listPrinted, true))
            PrintWeaponList();

        InitialiseWeaponsArray ();
        hash = GetNewWeaponForWeapon (hash, weap);
    }

    /*******************************************************/
    inline static void *
    GetGiveWeaponFuncAddress ()
    {
        // TODO: This is wrong, change it - the order can change in any update
        // causing it to not work anymore.
        hook::pattern pattern (
            "57 ? 83 ec 20 ? 8b ? 8b ? ? 8b ? e8 ? ? ? ? ? 8b e8 ? 85 c0 74 ?");

        return pattern.get (1).get<void *> (-15);
    }

    /*******************************************************/
    static void
    InitialiseRandomWeaponsHook ()
    {
        ReplaceJmpHook__fastcall<0xcc2d8, CInventoryItem *, CPedInventory*,
                                 uint32_t, uint32_t> (
            GetGiveWeaponFuncAddress (), RandomizeWeapon, CALLBACK_ORDER_BEFORE)
            .Activate ();

        // NOP JZ instruction to ensure that a weapon is always equipped by the
        // ped (weapons given by script only)
        *hook::get_pattern<uint16_t> ("0f 84 ? ? ? ? 80 7c ? ? 00 74 ? 8b", 11)
            = 0x9090;
    }

public:
    /*******************************************************/
    WeaponRandomizer ()
    {
        InitialiseAllComponents ();
        InitialiseRandomWeaponsHook ();
    }
} _weap;

std::vector<uint32_t>           WeaponRandomizer::mValidWeapons;
std::discrete_distribution<int> WeaponRandomizer::mDistribution;
