#pragma once

#include "scrThread.hh"

class CutSceneManager;
class CEntity;

/*******************************************************/
/* Cutscene and Script Fixes - Fixes required for Ped Randomizer to function
 * properly without softlocking the game in major ways. The fixes include
 * register entity fixes for cutscene ped randomizer and returning the original
 * ped model for scripts. */
/*******************************************************/
class PedRandomizer_MainFixes
{
    static uint32_t FixupScriptEntityModel (uint32_t guid);
    static void     FixupScriptEntityModel (scrThread::Info *info);
    static void     FixIsPedModelNative (scrThread::Info *info);

    template <auto &CutSceneManager_RegisterEntity>
    static void CorrectRegisterEntity (CutSceneManager *mgr, CEntity *entity,
                                       uint32_t *handle, uint32_t *modelHash,
                                       bool p5, bool p6, bool p7, uint32_t p8);

    static void AdjustAiBlipNoticeRange (scrThread::Info *info);
    static void AdjustAiBlipForcedOn (scrThread::Info *info);

    static void Nop (scrThread::Info *info);

public:
    static void Initialise ();
};
