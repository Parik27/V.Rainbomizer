#include <Utils.hh>
#include <common/common.hh>
#include <common/config.hh>
#include "CHandlingDataMgr.hh"
#include <common/parser.hh>

using CHandlingDataRandomizer = ParserRandomHelper<
    CHandlingData,
    RandomizedFieldsWrapper<
        ShuffleRandomizer<float>, "fMass"_joaat, "fInitialDragCoeff"_joaat,
        "fPercentSubmerged"_joaat, "fDriveBiasFront"_joaat,
        "nInitialDriveGears"_joaat, "fInitialDriveForce"_joaat,
        "fDriveInertia"_joaat, "fClutchChangeRateScaleUpShift"_joaat,
        "fClutchChangeRateScaleDownShift"_joaat,
        "fInitialDriveMaxFlatVel"_joaat, "fBrakeForce"_joaat,
        "fBrakeBiasFront"_joaat, "fHandBrakeForce"_joaat, "fSteeringLock"_joaat,
        "fTractionCurveMax"_joaat, "fTractionCurveMin"_joaat,
        "fTractionCurveLateral"_joaat, "fTractionSpringDeltaMax"_joaat,
        "fLowSpeedTractionLossMult"_joaat, "fCamberStiffnesss"_joaat,
        "fTractionBiasFront"_joaat, "fTractionLossMult"_joaat,
        "fSuspensionForce"_joaat, "fSuspensionCompDamp"_joaat,
        "fSuspensionReboundDamp"_joaat, "fSuspensionRaise"_joaat,
        "fSuspensionBiasFront"_joaat, "fAntiRollBarForce"_joaat,
        "fAntiRollBarBiasFront"_joaat, "fCollisionDamageMult"_joaat,
        "fWeaponDamageMult"_joaat, "fDeformationDamageMult"_joaat,
        "fEngineDamageMult"_joaat, "fPetrolTankVolume"_joaat,
        "fOilVolume"_joaat, "fSeatOffsetDistX"_joaat, "fSeatOffsetDistY"_joaat,
        "fSeatOffsetDistZ"_joaat, "fRollCentreHeightFront"_joaat,
        "fRollCentreHeightRear"_joaat, "fSuspensionUpperLimit"_joaat,
        "fSuspensionLowerLimit"_joaat>>;

using CCarHandlingDataRandomizer = ParserRandomHelper<
    CCarHandlingData,
    RandomizedFieldsWrapper<ShuffleRandomizer<float>,
                            "fBackEndPopUpCarImpulseMult"_joaat,
                            "fBackEndPopUpBuildingImpulseMult"_joaat,
                            "fBackEndPopUpMaxDeltaSpeed"_joaat>>;

using CBikeHandlingDataRandomizer = ParserRandomHelper<
    CBikeHandlingData,
    RandomizedFieldsWrapper<
        ShuffleRandomizer<float>, "fLeanFwdCOMMult"_joaat,
        "fLeanFwdForceMult"_joaat, "fLeanBakCOMMult"_joaat,
        "fLeanBakForceMult"_joaat, "fMaxBankAngle"_joaat,
        "fFullAnimAngle"_joaat, "fDesLeanReturnFrac"_joaat,
        "fStickLeanMult"_joaat, "fBrakingStabilityMult"_joaat,
        "fInAirSteerMult"_joaat, "fWheelieBalancePoint"_joaat,
        "fStoppieBalancePoint"_joaat, "fWheelieSteerMult"_joaat,
        "fRearBalanceMult"_joaat, "fFrontBalanceMult"_joaat,
        "fBikeGroundSideFrictionMult"_joaat,
        "fBikeWheelGroundSideFrictionMult"_joaat, "fBikeOnStandLeanAngle"_joaat,
        "fBikeOnStandSteerAngle"_joaat, "fJumpForce"_joaat>>;

using CBoatHandlingDataRandomizer = ParserRandomHelper<
    CBoatHandlingData,
    RandomizedFieldsWrapper<
        ShuffleRandomizer<float>, "fBoxFrontMult"_joaat, "fBoxRearMult"_joaat,
        "fBoxSideMult"_joaat, "fSampleTop"_joaat, "fSampleBottom"_joaat,
        "fAquaplaneForce"_joaat, "fAquaplanePushWaterMult"_joaat,
        "fAquaplanePushWaterCap"_joaat, "fAquaplanePushWaterApply"_joaat,
        "fRudderForce"_joaat, "fRudderOffsetSubmerge"_joaat,
        "fRudderOffsetForce"_joaat, "fRudderOffsetForceZMult"_joaat,
        "fWaveAudioMult"_joaat, "fDragCoefficient"_joaat,
        "fKeelSphereSize"_joaat, "fPropRadius"_joaat, "fLowLodAngOffset"_joaat,
        "fLowLodDraughtOffset"_joaat, "fImpellerOffset"_joaat,
        "fImpellerForceMult"_joaat, "fDinghySphereBuoyConst"_joaat,
        "fProwRaiseMult"_joaat, "fLook_L_R_CamHeight"_joaat>>;

using CVehicleWeaponHandlingDataRandomizer = ParserRandomHelper<
    CVehicleWeaponHandlingData,
    RandomizedFieldsWrapper<

        /* ArrayRandomizer<A,B> - uWeaponHash is an array type

           B - (atFixedArray<uint32_t, 4>) - The array is a fixed array with
           element type uint32_t and is 4 elements in length

           A - (SelectiveRandomizer<C, 0, 0xFFFFFFFF>) - Randomizer (C) will
           only randomize values that are NOT (0 or -1).

           ShuffleRandomizer<uint32_t> - Randomizer for uWeaponHash is a simple
           shuffle randomizer that will shuffle between all the possible values.
         */

        ArrayRandomizer<
            SelectiveRandomizer<ShuffleRandomizer<uint32_t>,
                                SelectionType::EXCLUDING, 0, 0xFFFFFFFF>,
            atFixedArray<uint32_t, 4>>,
        "uWeaponHash"_joaat>>;

using SubHandlingDataRandomizer = ParserRandomHelperContainer<
    CCarHandlingDataRandomizer, CBikeHandlingDataRandomizer,
    CBoatHandlingDataRandomizer, CVehicleWeaponHandlingDataRandomizer>;

class parTree;
class parManager;
class fiStream;

class HandlingRandomizer
{
    inline static CHandlingDataRandomizer   sm_HandlingDataRandomizer;
    inline static SubHandlingDataRandomizer sm_SubHandlingDataRandomizer;

    /*******************************************************/
    static void
    HandleSubHandlingRandomization (CHandlingData *data, bool sample)
    {

        for (auto &i : data->Get<atArray<CBaseSubHandlingData *>> (
                 "SubHandlingData"_joaat))
            {
                if (!i)
                    continue;

                auto name = i->parser_GetStructure ()->nName;

                if (!sample)
                    {
                        sm_SubHandlingDataRandomizer.RandomizeObject (i, name);
                        i->InitValues ();
                    }
                else
                    sm_SubHandlingDataRandomizer.AddSample (i, name);
            }
    }

    /*******************************************************/
    static CHandlingDataRandomizer &
    GetRandomizerForHandlingType (CHandlingData *data)
    {

        /* This function returns a different CHandlingDataRandomzier instance
         * for a CHandlingData type based on the sub handling datas it has. If
         * it has a bike or a boat sub handling data, it'll return a unique
         * randomizer for each of their types. So effectively this means a
         * unique randomizer for bikes and boats.  */

        // Default randomizer for all types except boats, bikes and trailers
        static CHandlingDataRandomizer Automobiles;

        for (auto &i : data->Get<atArray<CBaseSubHandlingData *>> (
                 "SubHandlingData"_joaat))
            {
                if (!i)
                    continue;

                auto name = i->parser_GetStructure ()->nName;
                switch (name)
                    {
                        // Bikes
                        case "CBikeHandlingData"_joaat: {
                            static CHandlingDataRandomizer Bikes;
                            return Bikes;
                        }

                        // Boats
                        case "CBoatHandlingData"_joaat: {
                            static CHandlingDataRandomizer Boats;
                            return Boats;
                        }

                        // Trailers
                        case "CTrailerHandlingData"_joaat: {
                            static CHandlingDataRandomizer Trailers;
                            return Trailers;
                        }
                    }
            }

        return Automobiles;
    }

    /*******************************************************/
    static void
    RandomizeHandling ()
    {
        for (auto &i : CHandlingDataMgr::sm_Instance->pData)
            {
                GetRandomizerForHandlingType (i).RandomizeObject (*i);
                HandleSubHandlingRandomization (i, false);

                i->InitValues ();
            }
    }

    /*******************************************************/
    template <auto &O>
    static parTree *
    AddRandomizationSamples (parManager *p1, fiStream *p2, parStructure *p3,
                             CHandlingDataMgr *out, bool p5, void *p6)
    {
        parTree *ret = O (p1, p2, p3, out, p5, p6);

        for (auto &i : out->pData)
            {
                GetRandomizerForHandlingType (i).AddSample (*i);
                HandleSubHandlingRandomization (i, true);
            }

        return ret;
    }

    /*******************************************************/
    void
    InitialiseCollectSamplesHooks ()
    {

        /* We need additional hooks here as opposed to just doing this in the
         * randomization function because the values are post processed after
         * being read from the parser, so there needs to be a hook that samples
         * them before they're post processed */

        auto InstallHook = [] (auto pattern, auto offset) {
            REGISTER_HOOK (pattern, offset, AddRandomizationSamples, parTree *,
                           parManager *, fiStream *, parStructure *,
                           CHandlingDataMgr *, bool, void *);
        };

        InstallHook ("8b d8 c6 44 ? ? 01 e8 ? ? ? ? ? 8b cb", 7);
        InstallHook ("89 ? ? ? 89 ? ? c6 44 ? ? 01 e8", 12);
    }

public:
    /*******************************************************/
    HandlingRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("HandlingRandomizer"))
            return;

        InitialiseAllComponents ();
        InitialiseCollectSamplesHooks ();

        Rainbomizer::Common::AddInitCallback ([] (bool session) {
            if (session)
                RandomizeHandling ();
        });
    }
} handling;
