#include <cstdint>
#include <random>
#include <algorithm>
#include <ctime>

#include <scrThread.hh>
#include <array>

#include "common/config.hh"
#include "common/common.hh"
#include "common/logger.hh"
#include "exceptions/exceptions_Mgr.hh"
#include "peds/peds_Compatibility.hh"

#include "weapons_equipMgr.hh"
#include "Utils.hh"
#include "CEntity.hh"
#include "CPed.hh"
#include "UtilsHooking.hh"
#include "rage.hh"
#include "CItemInfo.hh"

CInventoryItem *(*CPedInventory_GiveWeapon2d8) (CPedInventory *, uint32_t,
                                                uint32_t);
void (*SET_CURRENT_PED_WEAPON784) (uint32_t, uint32_t, bool);

class WeaponRandomizer
{
    static inline std::vector<uint32_t>           mValidWeapons;
    static inline std::discrete_distribution<int> mDistribution;
    static inline bool                            mSkipNextWeaponRandomization;
    static inline WeaponEquipMgr                  mEquipMgr;

    static inline std::unordered_map<uint32_t, uint32_t> mValidWeaponGroups;

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
                                probabilities[rage::atStringHash (model)]
                                    = probability;
                                sum_probability += probability;
                                total_probabilities++;
                            }
                    }
            }
        fclose (file);

        double base_probability = 0.01;

        std::vector<double> weights;
        for (auto i : mValidWeapons)
            {
                double weight = base_probability;

                if (auto *weapWeight = LookupMap (probabilities, i))
                    weight = *weapWeight;

                // Multiply groupWeight with the current weight
                if (auto *gW = LookupMap (probabilities, mValidWeaponGroups[i]))
                    weight *= (*gW);

                weights.push_back (weight);
            }

        mDistribution
            = std::discrete_distribution<int>{weights.begin (), weights.end ()};
    }

    /*******************************************************/
    static bool
    IsValidWeapon (CWeaponInfo &info)
    {
        if (info.Equate ("FireType"_joaat).ToHash () == "NONE"_joaat)
            return false;

        if (info.Get<uint32_t> ("HumanNameHash"_joaat) == "wt_invalid"_joaat)
            return false;

        if (info.Bitset ("WeaponFlags"_joaat)["Thrown"_joaat])
            return false;

        return true;
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

        for (auto &info : CWeaponInfoManager::sm_Instance->aItemInfos)
            {
                static_assert ("cweaponinfo"_joaat == 0x861905b4);
                uint32_t outHash = 0;

                if (info->Model && !DoesElementExist (mExceptions, info->Name)
                    && info->GetClassId (outHash) == "cweaponinfo"_joaat
                    && IsValidWeapon (*static_cast<CWeaponInfo *> (info)))
                    {
                        mValidWeapons.push_back (info->Name);
                        mValidWeaponGroups[info->Name]
                            = static_cast<CWeaponInfo *> (info)->Get<uint32_t> (
                                "Group"_joaat);
                    }
            }

        InitialiseWeaponWeights ();
        PrintWeaponList ();
        Rainbomizer::Logger::LogMessage ("Initialised %d valid weapons",
                                         mValidWeapons.size ());
    }

    /*******************************************************/
    static bool
    ShouldRandomizeWeapon (uint32_t weapon)
    {
        static const std::array Patterns
            = {std::tuple ("franklin1"_joaat, "weapon_assaultrifle"_joaat)};

        return scrThread::GetActiveThread ()
               && !DoesElementExist (Patterns,
                                     std::tuple (scrThread::GetActiveThread ()
                                                     ->m_Context.m_nScriptHash,
                                                 weapon));
    }

    /*******************************************************/
    static uint32_t
    GetNewWeaponForWeapon (uint32_t weapon, CPedInventory *weapons)
    {
        thread_local static std::mt19937 engine{(unsigned int) time (NULL)};

        // CPed Constructor AddWeapon?
        if (!weapons->m_pPed->m_pModelInfo || weapon == 0)
            return weapon;

        bool isPlayer
            = PedRandomizerCompatibility::GetOriginalModel (weapons->m_pPed)
                  ->m_bIsPlayerType;

        // Don't randomize the weapon if it's given to the player
        if (isPlayer)
            return weapon;

        if (DoesElementExist (mValidWeapons, weapon)
            && ShouldRandomizeWeapon (weapon))
            return mValidWeapons[mDistribution (engine)];

        return weapon;
    }

    /*******************************************************/
    static CInventoryItem *
    RandomizeWeapon (CPedInventory *weap, uint32_t hash, uint32_t ammo)
    {
        Rainbomizer::ExceptionHandlerMgr::GetInstance ().Init ();
        if (std::exchange (mSkipNextWeaponRandomization, false))
            return CPedInventory_GiveWeapon2d8 (weap, hash, ammo);

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
            }

        return CPedInventory_GiveWeapon2d8 (weap, hash, ammo);
    }

    /*******************************************************/
    static void
    CorrectSetCurrentWeapon (uint32_t ped, uint32_t weaponHash, bool equipNow)
    {

        SET_CURRENT_PED_WEAPON784 (ped, mEquipMgr.GetWeaponToEquip (weaponHash),
                                   true);
    }

    /*******************************************************/
    static void
    PrintWeaponList ()
    {
        for (auto &info : CWeaponInfoManager::sm_Instance->aItemInfos)
            {
                uint32_t outHash = 0;
                bool     valid
                    = info->Model
                      && info->GetClassId (outHash) == "cweaponinfo"_joaat
                      && IsValidWeapon (*static_cast<CWeaponInfo *> (info));

                Rainbomizer::Logger::LogMessage (
                    "classId => %x, modelId = %x, name = %x, value = %s",
                    info->GetClassId (outHash), info->Model, info->Name,
                    valid ? "true" : "false");
            }
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
        RegisterJmpHook<15> (GetGiveWeaponFuncAddress (),
                             CPedInventory_GiveWeapon2d8, RandomizeWeapon);

        // Hook to make the game make the ped equip the correct weapon.
        RegisterJmpHook<15> (
            "? 8b ? ? ? ? ? ? 85 c9 0f 84 ? ? ? ? ? 83 c1 18 8b d7", -37,
            SET_CURRENT_PED_WEAPON784, CorrectSetCurrentWeapon);

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
