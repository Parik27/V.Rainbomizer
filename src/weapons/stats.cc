#include "CEntity.hh"
#include "CItemInfo.hh"
#include "CModelInfo.hh"
#include "CStreaming.hh"
#include "Utils.hh"
#include "common/events.hh"
#include "common/config.hh"
#include "common/minhook.hh"
#include "common/parser.hh"
#include "common/logger.hh"
#include "scrThread.hh"
#include <cstdint>
#include <ctime>
#include <utility>

#include <CTheScripts.hh>

#ifdef ENABLE_DEBUG_MENU
#include <debug/base.hh>
#endif

using namespace NativeLiterals;

// Ammo Info Randomizer
// *******************************************************
using CAmmoInfoRandomizer = ParserRandomHelper<
    CAmmoInfo, RandomizedFieldsWrapper<
                   SelectiveRandomizer<ShuffleRandomizer<uint32_t>,
                                       SelectionType::EXCLUDING, 0>,
                   "AmmoMax"_joaat, "AmmoMax50"_joaat, "AmmoMax100"_joaat>>;

// Weapon Info Randomizer
// *******************************************************
using CWeaponInfoRandomizerBase = ParserRandomHelper<
    CWeaponInfo,
    RandomizedFieldsWrapper<
        ShuffleRandomizer<uint32_t>, "ClipSize"_joaat, "Audio"_joaat,
        "RecoilShakeHash"_joaat, "RecoilShakeHashFirstPerson"_joaat,
        "MinTimeBetweenRecoilShakes"_joaat, "HumanNameHash"_joaat>,
    RandomizedFieldsWrapper<
        ShuffleRandomizer<float>, "AccuracySpread"_joaat,
        "AccurateModeAccuracyModifier"_joaat, "RunAndGunAccuracyModifier"_joaat,
        "RecoilAccuracyMax"_joaat, "RecoilErrorTime"_joaat,
        "RecoilRecoveryRate"_joaat, "RecoilAccuracyToAllowHeadShotAI"_joaat,
        "MinHeadShotDistanceAI"_joaat, "MaxHeadShotDistanceAI"_joaat,
        "HeadShotDamageModifierAI"_joaat,
        "RecoilAccuracyToAllowHeadShotPlayer"_joaat,
        "MinHeadShotDistancePlayer"_joaat, "MaxHeadShotDistancePlayer"_joaat,
        "HeadShotDamageModifierPlayer"_joaat, "Damage"_joaat,
        "DamageTime"_joaat, "DamageTimeInVehicle"_joaat,
        "DamageTimeInVehicleHeadShot"_joaat, "HitLimbsDamageModifier"_joaat,
        "LightlyArmouredDamageModifier"_joaat, "Force"_joaat,
        "ForceHitPed"_joaat, "ForceHitVehicle"_joaat,
        "ForceHitFlyingHeli"_joaat, "ForceMaxStrengthMult"_joaat,
        "ForceFalloffRangeStart"_joaat, "ForceFalloffRangeEnd"_joaat,
        "ForceFalloffMin"_joaat, "Penetration"_joaat,
        "VerticalLaunchAdjustment"_joaat, "DropForwardVelocity"_joaat,
        "Speed"_joaat, "BatchSpread"_joaat, "BulletBendingNearRadius"_joaat,
        "BulletBendingFarRadius"_joaat, "BulletBendingZoomedRadius"_joaat,
        "FirstPersonBulletBendingFarRadius"_joaat,
        "FirstPersonBulletBendingZoomedRadius"_joaat, "WeaponRange"_joaat,
        "BulletDirectionOffsetInDegrees"_joaat, "AiSoundRange"_joaat,
        "AiPotentialBlastEventRange"_joaat, "DamageFallOffRangeMin"_joaat,
        "DamageFallOffRangeMax"_joaat, "DamageFallOffModifier"_joaat,
        "CameraFov"_joaat, "FirstPersonScopeFov"_joaat,
        "FirstPersonDofSubjectMagnificationPowerFactorNear"_joaat,
        "FirstPersonDofMaxNearInFocusDistance"_joaat,
        "FirstPersonDofMaxNearInFocusDistanceBlendLevel"_joaat,
        "ZoomFactorForAccurateMode"_joaat, "RecoilShakeAmplitude"_joaat,
        "ExplosionShakeAmplitude"_joaat, "IkRecoilDisplacement"_joaat,
        "IkRecoilDisplacementScope"_joaat,
        "IkRecoilDisplacementScaleBackward"_joaat,
        "IkRecoilDisplacementScaleVertical"_joaat>>;

using CWeaponInfoRandomizer
    = ParserRandomHelperContainerForEachFieldValue<CWeaponInfoRandomizerBase,
                                                   "FireType"_joaat>;

// Item Info Randomizer
// *******************************************************
using ItemInfoRandomizer
    = ParserRandomHelperContainer<CAmmoInfoRandomizer, CWeaponInfoRandomizer>;

class WeaponStatsRandomizer
{
    inline static char ModelFileName[] = "WeaponModels.txt";
    using ModelsRandomizer
        = DataFileBasedModelRandomizer<ModelFileName,
                                       CStreaming::GetModelByHash<>>;

    inline static ItemInfoRandomizer sm_ItemInfoRandomizer;
    inline static ModelsRandomizer   sm_WeaponModelRandomizer;

public:
    /*******************************************************/
    static bool
    HandleItemInfoRandomization (bool sample)
    {
        for (CItemInfo *i : CWeaponInfoManager::sm_Instance->aItemInfos)
            {
                if (i->Model != -1u && i->Model)
                    sm_WeaponModelRandomizer.RandomizeObject (i->Name,
                                                              i->Model);

                std::uint32_t hash;
                if (sample)
                    sm_ItemInfoRandomizer.AddSample (i, i->GetClassId (hash));
                else
                    sm_ItemInfoRandomizer.RandomizeObject (i, i->GetClassId (
                                                                  hash));
            }

        return true;
    }

    /*******************************************************/
    static void
    RandomizeWeaponStats (bool session)
    {
        if (!session)
            return;

        static bool sampled = HandleItemInfoRandomization (true);
        HandleItemInfoRandomization (false);
    }

    /*******************************************************/
    template <auto &CProjectileEntity__PreSim>
    static int
    ProjectilePhysicsFix (CEntity *p1, float p2, bool p3, int p4)
    {
        /* Fixes a crash caused by delayed loading of the physics object for a
         * rocket projectile. This adds a null pointer check that R* forgot ..
         */

        if (!p1->m_phInst)
            {
                Rainbomizer::Logger::LogMessage (
                    "Projectile Physics Instance not loaded: %x",
                    p1->m_pModelInfo->m_nHash);

                return 2;
            }

        return CProjectileEntity__PreSim (p1, p2, p3, p4);
    }

    /*******************************************************/
    static void
    FixShootsAtCoord (scrThread::Info *ctx)
    {
        auto ped = ctx->GetArg (0);

        uint32_t weap      = "GET_SELECTED_PED_WEAPON"_n(ped);
        auto    *weapModel = CStreaming::GetModelByHash (weap);

        static constexpr std::array BlackListedWeapons
            = {"prop_space_pistol"_joaat, "prop_space_rifle"_joaat,
               "ch_prop_collectibles_limb_01a"_joaat};

        // We know this can happen with these weapons, so exit without logging
        if (DoesElementExist (BlackListedWeapons, weap))
            return;

        // In all other cases (e.g., it gets called for melee weapons randomized
        // into props), exit with a log message at a cooldown (so as to not spam
        // the log more than necessary).
        if (!weapModel
            || weapModel->GetType () != eModelInfoType::MODEL_INFO_WEAPON)
            {
                const int   LOG_COOLDOWN = 5;
                static auto lastPrint    = 0;

                if (time (NULL) - lastPrint > LOG_COOLDOWN)
                    Rainbomizer::Logger::LogMessage (
                        "SET_PED_SHOOTS_AT_COORD called for non-weapon model: "
                        "%x %p",
                        weap, weapModel);
                return;
            }

        NativeManager::InvokeNative ("SET_PED_SHOOTS_AT_COORD"_joaat, ctx);
    }

    /*******************************************************/
    WeaponStatsRandomizer ()
    {

        if (!ConfigManager::ReadConfig ("WeaponStatsRandomizer"))
            return;

        // Randomize on game load
        Rainbomizer::Events ().OnInit += RandomizeWeaponStats;

#ifdef ENABLE_DEBUG_MENU
        DebugInterfaceManager::AddAction ("Randomize Weapon Stats",
                                          RandomizeWeaponStats);
#endif
        REGISTER_MH_HOOK (
            "? 0f 28 e1 ? 8b 41 30 b9 ff ff 00 00 66 39 48 18 75 ?", -0x56,
            ProjectilePhysicsFix, int, CEntity *, float, bool, int);

        InitialiseAllComponents ();
    }
} _stats;
