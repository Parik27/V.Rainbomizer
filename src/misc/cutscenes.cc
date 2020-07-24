#include <cstdio>
#include <Utils.hh>
#include <CutSceneManager.hh>
#include "common/logger.hh"
#include "common/common.hh"

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
                if (std::find (std::begin (i), std::end (i), model)
                    != std::end (i))
                    return i[RandomInt (i.size () - 1)];
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
                    rage::atStringHashLowercase (line));
            }

        return true;
    }

    /*******************************************************/
    static void
    RandomizeCutScene (parInstanceVisitor *visitor, cutfCutsceneFile2 *file)
    {
        for (int i = 0; i < file->pCutsceneObjects.Size; i++)
            {
                if (file->pCutsceneObjects.Data[i]->GetType ()
                    == eCutfObjectType::MODEL)
                    {
                        auto obj = static_cast<cutfModelObject *> (
                            file->pCutsceneObjects.Data[i]);
                        obj->StreamingName
                            = GetRandomModel (obj->StreamingName);
                    }
            }

        VisitTopLevelStructure_37027e (visitor, file);
    }

public:
    /*******************************************************/
    CutSceneRandomizer ()
    {
        InitialiseAllComponents ();

        if (InitialiseModelData ())
            RegisterHook ("8d ? ? 20 0f ba e8 10 89 44 ? ? e8", 12,
                          VisitTopLevelStructure_37027e, RandomizeCutScene);
    }
} _cuts;
