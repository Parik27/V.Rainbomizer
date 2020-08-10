#include <cstdio>
#include <Utils.hh>
#include <CutSceneManager.hh>
#include "common/logger.hh"
#include "common/common.hh"
#include "common/config.hh"

class parInstanceVisitor;

void (*VisitTopLevelStructure_37027e) (parInstanceVisitor *,
                                       cutfCutsceneFile2 *);

class CutSceneRandomizer
{
    /*******************************************************/
    static std::vector<std::vector<uint32_t>>&
    GetModelsList ()
    {
        static std::vector<std::vector<uint32_t>> mModels;
        return mModels;
    }

    /*******************************************************/
    static uint32_t
    GetRandomModel (uint32_t model)
    {
        for (const auto &i : GetModelsList ())
            {
                if (DoesElementExist (i, model))
                    return GetRandomElement (i);
            }

        return model;
    }

    /*******************************************************/
    static bool
    InitialiseModelData ()
    {
        FILE *modelsFile = Rainbomizer::Common::GetRainbomizerDataFile (
            "CutsceneModels.txt");
        GetModelsList ().clear ();

        if (!modelsFile)
            return false;

        char line[512] = {0};
        GetModelsList ().push_back ({});
        while (fgets (line, 512, modelsFile))
            {
                if (strlen (line) < 2)
                    {
                        GetModelsList ().push_back ({});
                        continue;
                    }

                line[strcspn (line, "\n")] = 0;
                GetModelsList ().back ().push_back (
                    rage::atStringHash (line));
            }

        return true;
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
                        }

                        // Lights
                    case eCutfObjectType::ANIMATED_LIGHT:
                        case eCutfObjectType::LIGHT: {
                            auto obj = static_cast<cutfLightObject *> (
                                file->pCutsceneObjects.Data[i]);

                            // Random Colour
                            obj->vColour.x = RandomFloat(1.0);
                            obj->vColour.y = RandomFloat(1.0);
                            obj->vColour.z = RandomFloat(1.0);
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
        if (!ConfigManager::GetConfigs().cutscenes.enabled)
            return;
        
        InitialiseAllComponents ();

        if (InitialiseModelData ())
            RegisterHook ("8d ? ? 20 0f ba e8 10 89 44 ? ? e8", 12,
                          VisitTopLevelStructure_37027e, RandomizeCutScene);
    }
} _cuts;
