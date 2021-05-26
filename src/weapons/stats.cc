#include "CItemInfo.hh"
#include "CStreaming.hh"
#include "Utils.hh"
#include "common/common.hh"
#include "common/config.hh"
#include "common/parser.hh"
#include "common/logger.hh"
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
                if (i->Model != -1 && i->Model)
                    sm_WeaponModelRandomizer.RandomizeObject (i->Model);

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
    WeaponStatsRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("WeaponStatsRandomizer"))
            return;

        // Randomize on game load
        Rainbomizer::Common::AddInitCallback ([] (bool session) {
            if (!session)
                return;

            static bool sampled = HandleItemInfoRandomization (true);
            HandleItemInfoRandomization (false);
        });

        InitialiseAllComponents ();
    }
} _stats;
