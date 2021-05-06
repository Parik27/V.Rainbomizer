#pragma once

#include "CPed.hh"
#include <cstdint>
#include <cstring>

// Original enum has different names
enum class eTaskType : uint16_t
{
    CTaskAdvance                                                         = 383,
    CTaskAffectSecondaryBehaviour                                        = 52,
    CTaskAggressiveRubberneck                                            = 147,
    CTaskAgitated                                                        = 320,
    CTaskAgitatedAction                                                  = 321,
    CTaskAimAndThrowProjectile                                           = 289,
    CTaskAimFromGround                                                   = 291,
    CTaskAimGun                                                          = 12,
    CTaskAimGunBlindFire                                                 = 304,
    CTaskAimGunFromCoverIntro                                            = 302,
    CTaskAimGunFromCoverOutro                                            = 303,
    CTaskAimGunOnFoot                                                    = 4,
    CTaskAimGunScripted                                                  = 296,
    CTaskAimGunVehicleDriveBy                                            = 295,
    CTaskAimSweep                                                        = 313,
    CTaskAmbientClips                                                    = 38,
    CTaskAmbientLookAtEvent                                              = 53,
    CTaskAmbulancePatrol                                                 = 68,
    CTaskAnimatedAttach                                                  = 434,
    CTaskAnimatedFallback                                                = 528,
    CTaskAnimatedHitByExplosion                                          = 387,
    CTaskArmy                                                            = 78,
    CTaskArrestPed                                                       = 62,
    CTaskArrestPed2                                                      = 63,
    CTaskBirdLocomotion                                                  = 273,
    CTaskBlendFromNM                                                     = 406,
    CTaskBoatChase                                                       = 370,
    CTaskBoatCombat                                                      = 371,
    CTaskBoatStrafe                                                      = 372,
    CTaskBomb                                                            = 431,
    CTaskBringVehicleToHalt                                              = 156,
    CTaskBusted                                                          = 64,
    CTaskCallPolice                                                      = 446,
    CTaskCarDrive                                                        = 157,
    CTaskCarDriveWander                                                  = 151,
    CTaskCarReactToVehicleCollision                                      = 93,
    CTaskCarReactToVehicleCollisionGetOut                                = 95,
    CTaskCarSetTempAction                                                = 155,
    CTaskCharge                                                          = 384,
    CTaskChat                                                            = 425,
    CTaskChatScenario                                                    = 104,
    CTaskClearLookAt                                                     = 29,
    CTaskClimbLadder                                                     = 1,
    CTaskClimbLadderFully                                                = 48,
    CTaskCloseVehicleDoorFromInside                                      = 164,
    CTaskCloseVehicleDoorFromOutside                                     = 168,
    CTaskCombat                                                          = 342,
    CTaskCombatAdditionalTask                                            = 308,
    CTaskCombatClosestTargetInArea                                       = 307,
    CTaskCombatFlank                                                     = 341,
    CTaskCombatMounted                                                   = 343,
    CTaskCombatRoll                                                      = 3,
    CTaskCombatSeekCover                                                 = 339,
    CTaskComplex                                                         = 12,
    CTaskComplexControlMovement                                          = 35,
    CTaskComplexEvasiveStep                                              = 137,
    CTaskComplexGetOffBoat                                               = 153,
    CTaskComplexGoToPointAndStandStillTimed                              = 208,
    CTaskComplexOnFire                                                   = 25,
    CTaskComplexStuckInAir                                               = 140,
    CTaskConfront                                                        = 322,
    CTaskControlVehicle                                                  = 169,
    CTaskCoupleScenario                                                  = 117,
    CTaskCover                                                           = 287,
    CTaskCower                                                           = 126,
    CTaskCowerScenario                                                   = 106,
    CTaskCrawl                                                           = 20,
    CTaskCrouch                                                          = 127,
    CTaskCrouchToggle                                                    = 327,
    CTaskCutScene                                                        = 440,
    CTaskDamageElectric                                                  = 26,
    CTaskDeadBodyScenario                                                = 107,
    CTaskDetonator                                                       = 432,
    CTaskDiveToGround                                                    = 443,
    CTaskDoNothing                                                       = 15,
    CTaskDraggedToSafety                                                 = 364,
    CTaskDraggingToSafety                                                = 363,
    CTaskDropDown                                                        = 51,
    CTaskDuckAndCover                                                    = 146,
    CTaskDyingDead                                                       = 97,
    CTaskEnterCover                                                      = 300,
    CTaskEnterVehicle                                                    = 160,
    CTaskEnterVehicleAlign                                               = 161,
    CTaskEnterVehicleSeat                                                = 163,
    CTaskEscapeBlast                                                     = 240,
    CTaskExhaustedFlee                                                   = 215,
    CTaskExitCover                                                       = 301,
    CTaskExitVehicle                                                     = 2,
    CTaskExitVehicleSeat                                                 = 167,
    CTaskFSMClone                                                        = 12,
    CTaskFall                                                            = 422,
    CTaskFallAndGetUp                                                    = 19,
    CTaskFallOver                                                        = 18,
    CTaskFirePatrol                                                      = 65,
    CTaskFishLocomotion                                                  = 278,
    CTaskFlightlessBirdLocomotion                                        = 274,
    CTaskFlyAway                                                         = 219,
    CTaskFlyToPoint                                                      = 228,
    CTaskFlyingWander                                                    = 229,
    CTaskFollowLeaderAnyMeans                                            = 226,
    CTaskFollowLeaderInFormation                                         = 223,
    CTaskFollowWaypointRecording                                         = 261,
    CTaskForceMotionState                                                = 45,
    CTaskGangPatrol                                                      = 77,
    CTaskGeneralSweep                                                    = 57,
    CTaskGetOffTrain                                                     = 179,
    CTaskGetOnTrain                                                      = 178,
    CTaskGetOutOfWater                                                   = 256,
    CTaskGetUp                                                           = 16,
    CTaskGetUpAndStandStill                                              = 17,
    CTaskGoToAndClimbLadder                                              = 47,
    CTaskGoToCarDoorAndStandStill                                        = 195,
    CTaskGoToPointAiming                                                 = 230,
    CTaskGoToPointAnyMeans                                               = 224,
    CTaskGoToScenario                                                    = 231,
    CTaskGrowlAndFlee                                                    = 216,
    CTaskGun                                                             = 290,
    CTaskHandsUp                                                         = 0,
    CTaskHeliChase                                                       = 373,
    CTaskHeliCombat                                                      = 374,
    CTaskHeliOrderResponse                                               = 66,
    CTaskHeliPassengerRappel                                             = 67,
    CTaskHelicopterStrafe                                                = 254,
    CTaskHitWall                                                         = 125,
    CTaskHumanLocomotion                                                 = 268,
    CTaskInCover                                                         = 309,
    CTaskInVehicleBasic                                                  = 150,
    CTaskInVehicleSeatShuffle                                            = 165,
    CTaskIntimidate                                                      = 323,
    CTaskInvestigate                                                     = 352,
    CTaskJump                                                            = 421,
    CTaskJumpVault                                                       = 420,
    CTaskLeaveAnyCar                                                     = 152,
    CTaskMelee                                                           = 128,
    CTaskMeleeActionResult                                               = 130,
    CTaskMeleeUpperbodyAnims                                             = 131,
    CTaskMobilePhone                                                     = 426,
    CTaskMotionAiming                                                    = 272,
    CTaskMotionAimingTransition                                          = 285,
    CTaskMotionBase                                                      = 12,
    CTaskMotionBasicLocomotionLowLod                                     = 269,
    CTaskMotionDiving                                                    = 280,
    CTaskMotionDrunk                                                     = 283,
    CTaskMotionInAutomobile                                              = 170,
    CTaskMotionInCover                                                   = 288,
    CTaskMotionInTurret                                                  = 174,
    CTaskMotionInVehicle                                                 = 173,
    CTaskMotionOnBicycle                                                 = 171,
    CTaskMotionOnBicycleController                                       = 172,
    CTaskMotionParachuting                                               = 282,
    CTaskMotionPed                                                       = 264,
    CTaskMotionPedLowLod                                                 = 265,
    CTaskMotionStrafing                                                  = 270,
    CTaskMotionSwimming                                                  = 281,
    CTaskMotionTennis                                                    = 271,
    CTaskMountThrowProjectile                                            = 190,
    CTaskMove                                                            = 12,
    CTaskMoveAchieveHeading                                              = 206,
    CTaskMoveBase                                                        = 12,
    CTaskMoveBeInFormation                                               = 242,
    CTaskMoveBetweenPointsScenario                                       = 103,
    CTaskMoveCircle                                                      = 344,
    CTaskMoveCombatMounted                                               = 345,
    CTaskMoveCrossRoadAtTrafficLights                                    = 244,
    CTaskMoveCrowdAroundLocation                                         = 243,
    CTaskMoveFaceTarget                                                  = 207,
    CTaskMoveFollowEntityOffset                                          = 259,
    CTaskMoveFollowNavMesh                                               = 238,
    CTaskMoveFollowPointRoute                                            = 209,
    CTaskMoveGetOntoMainNavMesh                                          = 251,
    CTaskMoveGoToPoint                                                   = 205,
    CTaskMoveGoToPointAndStandStill                                      = 208,
    CTaskMoveGoToPointOnRoute                                            = 239,
    CTaskMoveGoToPointRelativeToEntityAndStandStill                      = 253,
    CTaskMoveGoToPointStandStillAchieveHeading                           = 246,
    CTaskMoveGoToVehicleDoor                                             = 196,
    CTaskMoveInAir                                                       = 39,
    CTaskMoveMeleeMovement                                               = 129,
    CTaskMovePlayer                                                      = 5,
    CTaskMoveScripted                                                    = 133,
    CTaskMoveSeekEntity_CEntitySeekPosCalculatorLastNavMeshIntersection  = 211,
    CTaskMoveSeekEntity_CEntitySeekPosCalculatorLastNavMeshIntersection2 = 212,
    CTaskMoveSeekEntity_CEntitySeekPosCalculatorStandard                 = 210,
    CTaskMoveSeekEntity_CEntitySeekPosCalculatorXYOffsetFixed            = 213,
    CTaskMoveSeekEntity_CEntitySeekPosCalculatorXYOffsetFixed2           = 214,
    CTaskMoveSequence                                                    = 36,
    CTaskMoveSlideToCoord                                                = 252,
    CTaskMoveStandStill                                                  = 34,
    CTaskMoveToTacticalPoint                                             = 385,
    CTaskMoveTrackingEntity                                              = 237,
    CTaskMoveWaitForTraffic                                              = 245,
    CTaskMoveWalkRoundVehicle                                            = 142,
    CTaskMoveWander                                                      = 241,
    CTaskMoveWithinAttackWindow                                          = 366,
    CTaskMoveWithinDefensiveArea                                         = 367,
    CTaskNMBalance                                                       = 396,
    CTaskNMBehaviour                                                     = 12,
    CTaskNMBrace                                                         = 391,
    CTaskNMBuoyancy                                                      = 392,
    CTaskNMControl                                                       = 407,
    CTaskNMDangle                                                        = 408,
    CTaskNMDraggingToSafety                                              = 413,
    CTaskNMDrunk                                                         = 412,
    CTaskNMElectrocute                                                   = 397,
    CTaskNMExplosion                                                     = 399,
    CTaskNMFallDown                                                      = 405,
    CTaskNMFlinch                                                        = 403,
    CTaskNMGenericAttach                                                 = 411,
    CTaskNMHighFall                                                      = 395,
    CTaskNMInjuredOnGround                                               = 393,
    CTaskNMJumpRollFromRoadVehicle                                       = 402,
    CTaskNMOnFire                                                        = 400,
    CTaskNMPose                                                          = 390,
    CTaskNMPrototype                                                     = 398,
    CTaskNMRelax                                                         = 388,
    CTaskNMRiverRapids                                                   = 415,
    CTaskNMScriptControl                                                 = 401,
    CTaskNMShot                                                          = 394,
    CTaskNMSimple                                                        = 416,
    CTaskNMSit                                                           = 404,
    CTaskNMThroughWindscreen                                             = 414,
    CTaskNavBase                                                         = 12,
    CTaskNetworkClone                                                    = 40,
    CTaskOpenDoor                                                        = 54,
    CTaskOpenVehicleDoorFromOutside                                      = 162,
    CTaskParachute                                                       = 334,
    CTaskParachuteObject                                                 = 335,
    CTaskPatrol                                                          = 354,
    CTaskPause                                                           = 14,
    CTaskPlaneChase                                                      = 377,
    CTaskPlayerDrive                                                     = 159,
    CTaskPlayerIdles                                                     = 10,
    CTaskPlayerOnFoot                                                    = 6,
    CTaskPlayerWeapon                                                    = 9,
    CTaskPolice                                                          = 58,
    CTaskPoliceOrderResponse                                             = 59,
    CTaskPoliceWantedResponse                                            = 69,
    CTaskPursueCriminal                                                  = 60,
    CTaskQuadLocomotion                                                  = 279,
    CTaskRageRagdoll                                                     = 417,
    CTaskRappel                                                          = 49,
    CTaskReactAimWeapon                                                  = 424,
    CTaskReactAndFlee                                                    = 444,
    CTaskReactInDirection                                                = 447,
    CTaskReactToBeingAskedToLeaveVehicle                                 = 176,
    CTaskReactToBeingJacked                                              = 175,
    CTaskReactToBuddyShot                                                = 448,
    CTaskReactToDeadPed                                                  = 427,
    CTaskReactToExplosion                                                = 441,
    CTaskReactToGunAimedAt                                               = 144,
    CTaskReactToImminentExplosion                                        = 442,
    CTaskReactToPursuit                                                  = 122,
    CTaskReloadGun                                                       = 298,
    CTaskRepositionMove                                                  = 284,
    CTaskRevive                                                          = 328,
    CTaskRideTrain                                                       = 180,
    CTaskSayAudio                                                        = 114,
    CTaskScenario                                                        = 12,
    CTaskScenarioFlee                                                    = 217,
    CTaskScriptedAnimation                                               = 134,
    CTaskSearch                                                          = 346,
    CTaskSearchBase                                                      = 12,
    CTaskSearchForUnknownThreat                                          = 429,
    CTaskSearchInAutomobile                                              = 348,
    CTaskSearchInBoat                                                    = 349,
    CTaskSearchInHeli                                                    = 350,
    CTaskSearchInVehicleBase                                             = 12,
    CTaskSearchOnFoot                                                    = 347,
    CTaskSeekEntityAiming                                                = 233,
    CTaskSeparate                                                        = 358,
    CTaskSetAndGuardArea                                                 = 356,
    CTaskSetBlockingOfNonTemporaryEvents                                 = 44,
    CTaskSetCharDecisionMaker                                            = 30,
    CTaskSetPedDefensiveArea                                             = 31,
    CTaskSetPedInVehicle                                                 = 197,
    CTaskSetPedOutOfVehicle                                              = 198,
    CTaskSharkAttack                                                     = 319,
    CTaskSharkCircle                                                     = 318,
    CTaskShellShocked                                                    = 369,
    CTaskShockingEvent                                                   = 12,
    CTaskShockingEventBackAway                                           = 86,
    CTaskShockingEventGoto                                               = 82,
    CTaskShockingEventHurryAway                                          = 83,
    CTaskShockingEventReact                                              = 85,
    CTaskShockingEventReactToAircraft                                    = 84,
    CTaskShockingEventStopAndStare                                       = 88,
    CTaskShockingEventThreatResponse                                     = 90,
    CTaskShockingEventWatch                                              = 80,
    CTaskShockingNiceCarPicture                                          = 89,
    CTaskShockingPoliceInvestigate                                       = 87,
    CTaskShootAtTarget                                                   = 355,
    CTaskShootOutTire                                                    = 368,
    CTaskShove                                                           = 324,
    CTaskShovePed                                                        = 55,
    CTaskShoved                                                          = 325,
    CTaskSidestep                                                        = 445,
    CTaskSlideToCoord                                                    = 234,
    CTaskSlopeScramble                                                   = 46,
    CTaskSmartFlee                                                       = 218,
    CTaskSmashCarWindow                                                  = 204,
    CTaskStandGuard                                                      = 357,
    CTaskStandGuardFSM                                                   = 353,
    CTaskStayInCover                                                     = 359,
    CTaskStealVehicle                                                    = 121,
    CTaskStealthKill                                                     = 381,
    CTaskSubmarineChase                                                  = 376,
    CTaskSubmarineCombat                                                 = 375,
    CTaskSwapWeapon                                                      = 56,
    CTaskSwat                                                            = 70,
    CTaskSwatFollowInLine                                                = 75,
    CTaskSwatGoToStagingArea                                             = 74,
    CTaskSwatOrderResponse                                               = 73,
    CTaskSwatWantedResponse                                              = 72,
    CTaskSwimmingWander                                                  = 235,
    CTaskSynchronizedScene                                               = 135,
    CTaskTakeOffHelmet                                                   = 92,
    CTaskTakeOffPedVariation                                             = 336,
    CTaskTargetUnreachable                                               = 378,
    CTaskTargetUnreachableInExterior                                     = 380,
    CTaskTargetUnreachableInInterior                                     = 379,
    CTaskThreatResponse                                                  = 351,
    CTaskThrowProjectile                                                 = 286,
    CTaskToHurtTransit                                                   = 386,
    CTaskTrainBase                                                       = 12,
    CTaskTriggerLookAt                                                   = 28,
    CTaskTryToGrabVehicleDoor                                            = 177,
    CTaskTurnToFaceEntityOrCoord                                         = 225,
    CTaskUnalerted                                                       = 120,
    CTaskUseClimbOnRoute                                                 = 41,
    CTaskUseDropDownOnRoute                                              = 42,
    CTaskUseLadderOnRoute                                                = 43,
    CTaskUseScenario                                                     = 118,
    CTaskUseSequence                                                     = 32,
    CTaskUseVehicleScenario                                              = 119,
    CTaskVariedAimPose                                                   = 365,
    CTaskVault                                                           = 50,
    CTaskVehicleAnimation                                                = 505,
    CTaskVehicleApproach                                                 = 463,
    CTaskVehicleAttack                                                   = 484,
    CTaskVehicleAttackTank                                               = 485,
    CTaskVehicleBlock                                                    = 476,
    CTaskVehicleBlockBackAndForth                                        = 479,
    CTaskVehicleBlockBrakeInFront                                        = 478,
    CTaskVehicleBlockCruiseInFront                                       = 477,
    CTaskVehicleBoostUseSteeringAngle                                    = 521,
    CTaskVehicleBrake                                                    = 514,
    CTaskVehicleBurnout                                                  = 523,
    CTaskVehicleChase                                                    = 362,
    CTaskVehicleCircle                                                   = 486,
    CTaskVehicleCombat                                                   = 360,
    CTaskVehicleConvertibleRoof                                          = 506,
    CTaskVehicleCrash                                                    = 480,
    CTaskVehicleCruiseBoat                                               = 467,
    CTaskVehicleCruiseNew                                                = 466,
    CTaskVehicleDeadDriver                                               = 465,
    CTaskVehicleEscort                                                   = 490,
    CTaskVehicleFSM                                                      = 12,
    CTaskVehicleFlee                                                     = 471,
    CTaskVehicleFleeAirborne                                             = 472,
    CTaskVehicleFleeBoat                                                 = 473,
    CTaskVehicleFlyDirection                                             = 519,
    CTaskVehicleFollow                                                   = 475,
    CTaskVehicleFollowRecording                                          = 474,
    CTaskVehicleFollowWaypointRecording                                  = 508,
    CTaskVehicleGoForward                                                = 517,
    CTaskVehicleGoTo                                                     = 12,
    CTaskVehicleGoToAutomobileNew                                        = 453,
    CTaskVehicleGoToBoat                                                 = 457,
    CTaskVehicleGoToHelicopter                                           = 455,
    CTaskVehicleGoToNavmesh                                              = 509,
    CTaskVehicleGoToPlane                                                = 454,
    CTaskVehicleGoToPointAutomobile                                      = 458,
    CTaskVehicleGoToPointWithAvoidanceAutomobile                         = 459,
    CTaskVehicleGoToSubmarine                                            = 456,
    CTaskVehicleGotoLongRange                                            = 511,
    CTaskVehicleGun                                                      = 200,
    CTaskVehicleHandBrake                                                = 515,
    CTaskVehicleHeadonCollision                                          = 520,
    CTaskVehicleHeliProtect                                              = 491,
    CTaskVehicleHover                                                    = 483,
    CTaskVehicleLand                                                     = 481,
    CTaskVehicleLandPlane                                                = 482,
    CTaskVehicleMissionBase                                              = 12,
    CTaskVehicleMountedWeapon                                            = 199,
    CTaskVehicleNoDriver                                                 = 504,
    CTaskVehicleParkNew                                                  = 507,
    CTaskVehiclePassengerExit                                            = 470,
    CTaskVehiclePersuit                                                  = 361,
    CTaskVehiclePlaneChase                                               = 503,
    CTaskVehiclePlayerDriveAutogyro                                      = 500,
    CTaskVehiclePlayerDriveAutomobile                                    = 493,
    CTaskVehiclePlayerDriveBike                                          = 494,
    CTaskVehiclePlayerDriveBoat                                          = 495,
    CTaskVehiclePlayerDriveDiggerArm                                     = 501,
    CTaskVehiclePlayerDriveHeli                                          = 499,
    CTaskVehiclePlayerDrivePlane                                         = 498,
    CTaskVehiclePlayerDriveSubmarine                                     = 496,
    CTaskVehiclePlayerDriveSubmarineCar                                  = 497,
    CTaskVehiclePlayerDriveTrain                                         = 502,
    CTaskVehiclePoliceBehaviour                                          = 487,
    CTaskVehiclePoliceBehaviourBoat                                      = 489,
    CTaskVehiclePoliceBehaviourHelicopter                                = 488,
    CTaskVehicleProjectile                                               = 201,
    CTaskVehiclePullAlongside                                            = 526,
    CTaskVehiclePullOver                                                 = 469,
    CTaskVehiclePursue                                                   = 460,
    CTaskVehicleRam                                                      = 461,
    CTaskVehicleReactToCopSiren                                          = 510,
    CTaskVehicleRevEngine                                                = 524,
    CTaskVehicleReverse                                                  = 513,
    CTaskVehicleShotTire                                                 = 522,
    CTaskVehicleSpinOut                                                  = 462,
    CTaskVehicleStop                                                     = 468,
    CTaskVehicleSurfaceInSubmarine                                       = 525,
    CTaskVehicleSwerve                                                   = 518,
    CTaskVehicleTempAction                                               = 12,
    CTaskVehicleThreePointTurn                                           = 464,
    CTaskVehicleTransformToSubmarine                                     = 527,
    CTaskVehicleTurn                                                     = 516,
    CTaskVehicleWait                                                     = 512,
    CTaskWaitForSteppingOut                                              = 116,
    CTaskWalkAway                                                        = 220,
    CTaskWalkRoundCarWhileWandering                                      = 138,
    CTaskWalkRoundEntity                                                 = 141,
    CTaskWander                                                          = 221,
    CTaskWanderInArea                                                    = 222,
    CTaskWanderingInRadiusScenario                                       = 101,
    CTaskWanderingScenario                                               = 100,
    CTaskWeapon                                                          = 8,
    CTaskWeaponBlocked                                                   = 299,
    CTaskWitness                                                         = 76,
    CTaskWrithe                                                          = 382
};

struct TaskTargetInfo
{
    Vector4 vecPos;
    int32_t field_0x10;
    CPed *  pedTarget;
};

struct aiTask
{
    void *    field_0x8;
    CPed *    pPed;
    aiTask *  parentTask;
    aiTask *  subTask;
    aiTask *  subTask2;
    uint8_t   m_nFlags[4];
    eTaskType nTaskId;
    uint8_t   nFSMState;
    uint8_t   field_0x37;
    float     field_0x38;
    float     field_0x3c;

    virtual void      Destructor ();
    virtual eTaskType GetType ();
    virtual aiTask *  Clone ();
    virtual void      MakeAbortable ();
    //...

    void DoSetState (uint32_t newState);
    void SetSubTask (aiTask *subTask);

    static void InitialisePatterns ();
};

class TaskHelper
{
    inline static uint8_t m_ScratchSpace[1024] = {0};

    template <typename T>
    static void
    SetScratchSpace (uint32_t offset, T data)
    {
        memcpy (m_ScratchSpace + offset, &data, sizeof (data));
    }

    static void
    ResetScratchSpace ()
    {
        memset (m_ScratchSpace, 0, 1024);
    }

public:
    template <typename T, uint32_t... Offsets, typename... Data>
    static T *CreateTask [[nodiscard]] (Data... data)
    {
        ResetScratchSpace ();
        *reinterpret_cast<void **> (m_ScratchSpace) = T::vftable;
        (..., SetScratchSpace (Offsets, data));

        return static_cast<T *> (
            reinterpret_cast<aiTask *> (m_ScratchSpace)->Clone ());
    }
};

struct CTaskAimAndThrowProjectile : public aiTask
{
public:
    inline static void *vftable;

    static auto *Create [[nodiscard]] (const TaskTargetInfo &target)
    {
        return TaskHelper::CreateTask<CTaskAimAndThrowProjectile, 176> (target);
    }
};

struct CTaskShootAtTarget : public aiTask
{
    uint8_t field_0x40[80];

public:
    TaskTargetInfo targetInfo;

    inline static void *vftable;
};

class CTaskCombat : public aiTask
{
public:
    inline static void *vftable;
};

class CTaskWeapon : public aiTask
{
public:
    inline static void *vftable;

    static auto *Create [[nodiscard]] (const TaskTargetInfo &target)
    {
        return TaskHelper::CreateTask<CTaskAimAndThrowProjectile, 160> (target);
    }
};

class CTaskGun : public aiTask
{
public:
    inline static void *vftable;

    uint8_t        field_0x40[160];
    TaskTargetInfo targetInfo;
};
