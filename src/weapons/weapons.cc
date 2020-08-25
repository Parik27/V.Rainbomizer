#include "Utils.hh"
#include "common/common.hh"
#include "common/logger.hh"
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
#include "common/config.hh"
#include "weapons_equipMgr.hh"
#include "exceptions/exceptions_Mgr.hh"

class WeaponRandomizer
{
    static inline std::vector<uint32_t>           mValidWeapons;
    static inline std::discrete_distribution<int> mDistribution;
    static inline bool                            mSkipNextWeaponRandomization;
    static inline WeaponEquipMgr                  mEquipMgr;

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
                                probabilities[rage::atStringHash (
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
        if (mValidWeapons.size ())
            return;

        mValidWeapons.clear ();

        // These models are exempt from randomization
        const std::vector<uint32_t> mExceptions = {};

        for (int i = 0; i < CWeaponInfoManager::sm_Instance->m_nInfosCount; i++)
            {
                auto     info      = CWeaponInfoManager::GetInfoFromIndex (i);
                uint32_t modelHash = info->GetModelHash ();

                static_assert ("cweaponinfo"_joaat == 0x861905b4);

                if (modelHash && !DoesElementExist (mExceptions, info->Name)
                    && info->GetClassId () == "cweaponinfo"_joaat)
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
        Rainbomizer::Logger::LogMessage (
            "{%s}: %x", scrThread::GetActiveThread ()->m_szScriptName, weapon);
        return false;
    }

    /*******************************************************/
    static uint32_t
    GetNewWeaponForWeapon (uint32_t weapon, CPedInventory *weapons)
    {
        thread_local static std::mt19937 engine{(unsigned int) time (NULL)};

        // CPed Constructor AddWeapon?
        if (!weapons->m_pPed->m_pModelInfo || weapon == 0)
            return weapon;

        bool isPlayer = ((CPedModelInfo *) weapons->m_pPed->m_pModelInfo)
                            ->m_bIsPlayerType;

        // Don't randomize the weapon if it's given to the player
        if (isPlayer)
            return weapon;

        if (DoesElementExist (mValidWeapons, weapon))
            return mValidWeapons[mDistribution (engine)];

        return weapon;
    }

    /*******************************************************/
    static bool
    RandomizeWeapon (CPedInventory *weap, uint32_t &hash, uint32_t ammo)
    {
        if (std::exchange (mSkipNextWeaponRandomization, false))
            return true;

        InitialiseWeaponsArray ();

        uint32_t originalHash = hash;

        hash = GetNewWeaponForWeapon (hash, weap);
        mEquipMgr.AddWeaponToEquip (hash, originalHash);

        // To prevent peds from fleeing
        if (weap->m_pPed->GetIntelligence ())
            {
                auto &behaviourFlags = weap->m_pPed->GetIntelligence ()
                                           ->GetCombatBehaviourFlags ();

                behaviourFlags.Set (BF_CanFightArmedPedsWhenNotArmed, true);
                behaviourFlags.Set (BF_AlwaysFight, true);
                behaviourFlags.Set (BF_CanThrowSmokeGrenade, true);

                for (int i = 0; i < BF_TOTAL_FLAGS; i++)
                    {
                        if (i != BF_AlwaysFlee)
                            behaviourFlags.Set(i, true);
                    }
            }

        return true;
    }

    /*******************************************************/
    static bool
    CorrectSetCurrentWeapon (uint32_t ped, uint32_t &weaponHash, bool &equipNow)
    {        
        Rainbomizer::ExceptionHandlerMgr::GetInstance ().Init ();
        
        uint32_t oriHash = weaponHash;
        weaponHash       = mEquipMgr.GetWeaponToEquip (weaponHash);

        equipNow = true;
        return true;
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
        // Hook to actually replace the weapons
        ReplaceJmpHook__fastcall<0xcc2d8, CInventoryItem *, CPedInventory *,
                                 uint32_t, uint32_t> (
            GetGiveWeaponFuncAddress (), RandomizeWeapon)
            .Activate ();

        // Hook to make the game make the ped equip the correct weapon.
        ReplaceJmpHook__fastcall<0xff8a784, void, uint32_t, uint32_t, bool> (
            hook::get_pattern (
                "? 8b ? ? ? ? ? ? 85 c9 0f 84 ? ? ? ? ? 83 c1 18 8b d7", -37),
            CorrectSetCurrentWeapon)
            .Activate ();

        // NOP JZ instruction to ensure that a weapon is always equipped by the
        // ped (weapons given by script only)
        injector::WriteMemory<uint16_t> (
            hook::get_pattern ("0f 84 ? ? ? ? 80 7c ? ? 00 74 ? 8b", 11),
            0x9090, true);
    }

public:
    /*******************************************************/
    WeaponRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("WeaponRandomizer"))
            return;

        InitialiseAllComponents ();
        InitialiseRandomWeaponsHook ();
    }
} _weap;
