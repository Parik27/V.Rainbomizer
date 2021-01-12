#include "CutSceneManager.hh"
#include <Utils.hh>

/*******************************************************/
void
cutfCutsceneFile2::InitialisePatterns ()
{
    // ? 89 0d ? ? ? ? ? 81 c1 b8 02 00 00 e8
    sm_pCurrentFile = GetRelativeReference<cutfCutsceneFile2 *> (
        "89 0d ? ? ? ? ? 81 c1 b8 02 00 00 e8", 3, 7);
}

cutfCutsceneFile2 **cutfCutsceneFile2::sm_pCurrentFile;
