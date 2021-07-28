#include "peds_MainFixes.hh"
#include "peds_Compatibility.hh"
#include "peds.hh"

#include <CPed.hh>
#include <Utils.hh>
#include <CTheScripts.hh>
#include <CutSceneManager.hh>

using namespace NativeLiterals;

using PR = PedRandomizer_Components;

/*******************************************************/
void
PedRandomizer_MainFixes::Nop (scrThread::Info *info)
{
}

/*******************************************************/
void
PedRandomizer_MainFixes::AdjustAiBlipForcedOn (scrThread::Info *info)
{
    info->GetArg<bool> (1) = true;
}
void
PedRandomizer_MainFixes::AdjustAiBlipNoticeRange (scrThread::Info *info)
{
    info->GetArg<float> (1) = -1.0f;
}

/*******************************************************/
void
PedRandomizer_MainFixes::FixIsPedModelNative (scrThread::Info *info)
{
    info->GetReturn ()
        = FixupScriptEntityModel (info->GetArg (0)) == info->GetArg (1);
}

/*******************************************************/
void
PedRandomizer_MainFixes::FixupScriptEntityModel (scrThread::Info *info)
{
    info->GetReturn () = FixupScriptEntityModel (info->GetArg (0));
}

/*******************************************************/
uint32_t
PedRandomizer_MainFixes::FixupScriptEntityModel (uint32_t guid)
{
    CPed *ped = static_cast<CPed *> (fwScriptGuid::GetBaseFromGuid (guid));

    if (!ped || !ped->m_pModelInfo)
        return 0;

    return PedRandomizer_Compatibility::GetOriginalModel (ped)->m_nHash;
}

/*******************************************************/
template <auto &CutSceneManager_RegisterEntity>
void
PedRandomizer_MainFixes::CorrectRegisterEntity (CutSceneManager *mgr,
                                                CEntity *        entity,
                                                uint32_t *       handle,
                                                uint32_t *modelHash, bool p5,
                                                bool p6, bool p7, uint32_t p8)
{
    if (entity
        && entity->m_pModelInfo->GetType () == eModelInfoType::MODEL_INFO_PED)
        {
            *modelHash = PedRandomizer_Compatibility::GetOriginalModel (
                             static_cast<CPed *> (entity))
                             ->m_nHash;
        }

    CutSceneManager_RegisterEntity (mgr, entity, handle, modelHash, p5, p6, p7,
                                    p8);
}

/*******************************************************/
void
PedRandomizer_MainFixes::Initialise ()
{
    // Fix for scripts where a certain hash is required for a certain ped.
    "GET_ENTITY_MODEL"_n.Hook (FixupScriptEntityModel);
    "IS_PED_MODEL"_n.Hook (FixIsPedModelNative);

    // Some missions disable additional peds streaming leaving only 4 or so in
    // memory. This makes it so that doesn't happen.
    if (PR::Config ().EnableNoLowBudget)
        {
            "SET_PED_POPULATION_BUDGET"_n.Hook (Nop);
            "SET_REDUCE_PED_MODEL_BUDGET"_n.Hook (Nop);
        }

#define HOOK(native, func) NativeCallbackMgr::Add<native##_joaat, func, true> ()

    // With animals, it can sometimes be hard to pinpoint where a small animal
    // is. This makes it so blips are always visible.
    if (PR::Config ().EnableBlipsAlwaysVisible)
        {
            HOOK ("SET_PED_AI_BLIP_NOTICE_RANGE", AdjustAiBlipNoticeRange);
            HOOK ("SET_PED_AI_BLIP_FORCED_ON", AdjustAiBlipForcedOn);
        }

#undef HOOK

    REGISTER_HOOK (
        "c6 44 ? ? 00 ? 8d ? f0 ? 8d ? f4 ? 8b c8 e8 ? ? ? ? ? 8b 5c", 16,
        CorrectRegisterEntity, void, CutSceneManager *, CEntity *, uint32_t *,
        uint32_t *, bool, bool, bool, uint32_t);
}
