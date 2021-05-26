#include <array>
#include <cstdint>
#include <cstdio>
#include <Utils.hh>
#include <CutSceneManager.hh>
#include "CModelInfo.hh"
#include "CStreaming.hh"
#include "Patterns/Patterns.hh"
#include "common/logger.hh"
#include "common/common.hh"
#include "common/config.hh"
#include "common/parser.hh"
#include "injector/injector.hpp"
#include "CPed.hh"
#include "peds/peds_Compatibility.hh"

class parInstanceVisitor;

void (*VisitTopLevelStructure_37027e) (parInstanceVisitor *,
                                       cutfCutsceneFile2 *);

class CutSceneRandomizer
{
    inline static char PropsFileName[] = "CutsceneModelsProps.txt";
    using PropsRandomizer
        = DataFileBasedModelRandomizer<PropsFileName,
                                       CStreaming::GetModelByHash<>>;

    inline static PropsRandomizer sm_Randomizer;

    /*******************************************************/
    static uint32_t
    GetRandomModel (uint32_t modelHash)
    {
        sm_Randomizer.RandomizeObject (modelHash);
        return modelHash;
    }

    /*******************************************************/
    static void
    RandomizeCutScene (parInstanceVisitor *visitor, cutfCutsceneFile2 *file)
    {
        for (int i = 0; i < file->pCutsceneObjects.Size; i++)
            {
                switch (file->pCutsceneObjects.Data[i]->GetType ())
                    {
                        // Cutscene Models
                        case eCutfObjectType::MODEL: {
                            auto obj = static_cast<cutfModelObject *> (
                                file->pCutsceneObjects.Data[i]);

                            obj->StreamingName
                                = GetRandomModel (obj->StreamingName);

                            break;
                        }

                        // Lights
                    case eCutfObjectType::ANIMATED_LIGHT:
                        case eCutfObjectType::LIGHT: {
                            auto obj = static_cast<cutfLightObject *> (
                                file->pCutsceneObjects.Data[i]);

                            // Random Colour
                            obj->vColour.x = RandomFloat (1.0);
                            obj->vColour.y = RandomFloat (1.0);
                            obj->vColour.z = RandomFloat (1.0);

                            break;
                        }

                    default: continue;
                    }
            }
        VisitTopLevelStructure_37027e (visitor, file);
    }

public:
    /*******************************************************/
    CutSceneRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("CutSceneRandomizer"))
            return;

        InitialiseAllComponents ();

        RegisterHook ("8d ? ? 20 0f ba e8 10 89 44 ? ? e8", 12,
                      VisitTopLevelStructure_37027e, RandomizeCutScene);
    }
} _cuts;
