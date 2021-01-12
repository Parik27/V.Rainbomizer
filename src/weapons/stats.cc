#include "CItemInfo.hh"
#include "Utils.hh"
#include "common/common.hh"
#include "common/config.hh"
#include "common/parser.hh"
#include <cstdint>
#include <utility>

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
        "BulletsInBatch"_joaat, "InitialRumbleDuration"_joaat,
        "RumbleDuration"_joaat, "InitialRumbleDurationFps"_joaat,
        "DefaultCameraHash"_joaat, "CoverCameraHash"_joaat,
        "RunAndGunCameraHash"_joaat, "CinematicShootingCameraHash"_joaat,
        "RecoilShakeHash"_joaat, "RecoilShakeHashFirstPerson"_joaat,
        "MinTimeBetweenRecoilShakes"_joaat, "HumanNameHash"_joaat>,

    RandomizedFieldsWrapper<
        SelectiveRandomizer<ShuffleRandomizer<uint32_t>,
                            SelectionType::EXCLUDING, 0, 0xFFFFFFFF>,
        "Model"_joaat>,

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
        "NetworkHitLimbsDamageModifier"_joaat,
        "LightlyArmouredDamageModifier"_joaat, "Force"_joaat,
        "ForceHitPed"_joaat, "ForceHitVehicle"_joaat,
        "ForceHitFlyingHeli"_joaat, "ForceMaxStrengthMult"_joaat,
        "ForceFalloffRangeStart"_joaat, "ForceFalloffRangeEnd"_joaat,
        "ForceFalloffMin"_joaat, "ProjectileForce"_joaat, "FragImpulse"_joaat,
        "Penetration"_joaat, "VerticalLaunchAdjustment"_joaat,
        "DropForwardVelocity"_joaat, "Speed"_joaat, "BatchSpread"_joaat,
        "ReloadTimeMP"_joaat, "ReloadTimeSP"_joaat, "VehicleReloadTime"_joaat,
        "AnimReloadRate"_joaat, "TimeBetweenShots"_joaat,
        "TimeLeftBetweenShotsWhereShouldFireIsCached"_joaat, "SpinUpTime"_joaat,
        "SpinTime"_joaat, "SpinDownTime"_joaat, "AlternateWaitTime"_joaat,
        "BulletBendingNearRadius"_joaat, "BulletBendingFarRadius"_joaat,
        "BulletBendingZoomedRadius"_joaat,
        "FirstPersonBulletBendingFarRadius"_joaat,
        "FirstPersonBulletBendingZoomedRadius"_joaat,
        "InitialRumbleIntensity"_joaat, "InitialRumbleIntensityTrigger"_joaat,
        "RumbleIntensity"_joaat, "RumbleIntensityTrigger"_joaat,
        "RumbleDamageIntensity"_joaat, "InitialRumbleIntensityFps"_joaat,
        "RumbleIntensityFps"_joaat, "NetworkPlayerDamageModifier"_joaat,
        "NetworkPedDamageModifier"_joaat,
        "NetworkHeadShotPlayerDamageModifier"_joaat, "LockOnRange"_joaat,
        "WeaponRange"_joaat, "BulletDirectionOffsetInDegrees"_joaat,
        "AiSoundRange"_joaat, "AiPotentialBlastEventRange"_joaat,
        "DamageFallOffRangeMin"_joaat, "DamageFallOffRangeMax"_joaat,
        "DamageFallOffModifier"_joaat, "CameraFov"_joaat,
        "FirstPersonScopeFov"_joaat,
        "FirstPersonDofSubjectMagnificationPowerFactorNear"_joaat,
        "FirstPersonDofMaxNearInFocusDistance"_joaat,
        "FirstPersonDofMaxNearInFocusDistanceBlendLevel"_joaat,
        "ZoomFactorForAccurateMode"_joaat, "RecoilShakeAmplitude"_joaat,
        "ExplosionShakeAmplitude"_joaat, "IkRecoilDisplacement"_joaat,
        "IkRecoilDisplacementScope"_joaat,
        "IkRecoilDisplacementScaleBackward"_joaat,
        "IkRecoilDisplacementScaleVertical"_joaat, "AimProbeLengthMin"_joaat,
        "AimProbeLengthMax"_joaat, "KillshotImpulseScale"_joaat,
        "AimingBreathingAdditiveWeight"_joaat,
        "FiringBreathingAdditiveWeight"_joaat,
        "StealthAimingBreathingAdditiveWeight"_joaat,
        "StealthFiringBreathingAdditiveWeight"_joaat,
        "AimingLeanAdditiveWeight"_joaat, "FiringLeanAdditiveWeight"_joaat,
        "StealthAimingLeanAdditiveWeight"_joaat,
        "StealthFiringLeanAdditiveWeight"_joaat,
        "ExpandPedCapsuleRadius"_joaat>>;

using CWeaponInfoRandomizer
    = ParserRandomHelperContainerForEachFieldValue<CWeaponInfoRandomizerBase,
                                                   "FireType"_joaat>;

// Item Info Randomizer
// *******************************************************
using ItemInfoRandomizer
    = ParserRandomHelperContainer<CAmmoInfoRandomizer, CWeaponInfoRandomizer>;

class WeaponStatsRandomizer
{
    inline static ItemInfoRandomizer sm_ItemInfoRandomizer;

public:
    /*******************************************************/
    static void
    HandleItemInfoRandomization (bool sample)
    {
        for (CItemInfo *i : CWeaponInfoManager::sm_Instance->aItemInfos)
            {
                std::uint32_t hash;
                if (sample)
                    sm_ItemInfoRandomizer.AddSample (i, i->GetClassId (hash));
                else
                    sm_ItemInfoRandomizer.RandomizeObject (i, i->GetClassId (
                                                                  hash));
            }
    }

    /*******************************************************/
    WeaponStatsRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("WeaponStatsRandomizer"))
            return;

        // Randomize on game load
        Rainbomizer::Common::AddInitCallback ([] (bool session) {
            if (!session)
                return;

            static bool toSample = true;

            if (std::exchange (toSample, false))
                HandleItemInfoRandomization (true);

            HandleItemInfoRandomization (false);
        });

        InitialiseAllComponents ();
    }
} _stats;
