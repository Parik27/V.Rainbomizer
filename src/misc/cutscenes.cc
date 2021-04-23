#include <array>
#include <cstdint>
#include <cstdio>
#include <Utils.hh>
#include <CutSceneManager.hh>
#include "Patterns/Patterns.hh"
#include "common/logger.hh"
#include "common/common.hh"
#include "common/config.hh"
#include "injector/injector.hpp"
#include "CPed.hh"

class parInstanceVisitor;

void (*VisitTopLevelStructure_37027e) (parInstanceVisitor *,
                                       cutfCutsceneFile2 *);

class CutSceneRandomizer
{
    inline static std::array<int32_t, 3> m_aPlayerObjects{};
    
    /*******************************************************/
    static std::vector<std::vector<uint32_t>> &
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
                GetModelsList ().back ().push_back (rage::atStringHash (line));
            }

        return true;
    }

    /*******************************************************/
    static int32_t
    GetPlayerArrayIdx (uint32_t hash)
    {
        switch (hash)
            {
            case "player_zero"_joaat: return 0; break;
            case "player_one"_joaat: return 1; break;
            case "player_two"_joaat: return 2; break;
            default: return -1;
            }
    }

    /*******************************************************/
    static void
    SetPlayerObjectIdFromObject (cutfModelObject *obj)
    {
        int idx = GetPlayerArrayIdx (obj->StreamingName);

        if (idx != -1)
            m_aPlayerObjects[idx] = obj->iObjectId;
    }

    /*******************************************************/
    static void
    RandomizeCutScene (parInstanceVisitor *visitor, cutfCutsceneFile2 *file)
    {
        for (auto &obj : m_aPlayerObjects)
            obj = -1;

        for (int i = 0; i < file->pCutsceneObjects.Size; i++)
            {
                switch (file->pCutsceneObjects.Data[i]->GetType ())
                    {
                        // Cutscene Models
                        case eCutfObjectType::MODEL: {
                            auto obj = static_cast<cutfModelObject *> (
                                file->pCutsceneObjects.Data[i]);

                            SetPlayerObjectIdFromObject(obj);
                            
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

    /*******************************************************/
    static void
    CorrectPlayerObjIdx (CutSceneManager *mgr)
    {
        int32_t arrIdx = GetPlayerArrayIdx (
            CPedFactory::Get ()->pPlayer->m_pModelInfo->m_nHash);

        if (m_aPlayerObjects[arrIdx] != -1)
            {
                mgr->bHasPlayerObjectId = true;
                mgr->m_nPlayerObjId     = m_aPlayerObjects[arrIdx];
            }
    }

public:
    /*******************************************************/
    CutSceneRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("CutSceneRandomizer"))
            return;

        InitialiseAllComponents ();

        if (!InitialiseModelData ())
            return;
        
        RegisterHook ("8d ? ? 20 0f ba e8 10 89 44 ? ? e8", 12,
                      VisitTopLevelStructure_37027e, RandomizeCutScene);

        //?? 8b d9 40 38 3d ?? ?? ?? ?? 75 ?? e8 ?? ?? ?? ??
        RegisterHook ("8b d9 40 38 3d ? ? ? ? 75 ? e8", 11,
                      CorrectPlayerObjIdx);

#if (false)
        // Disables the check for StreamingName of cutscene registered entities
        // pCVar5>m_nStreamingName == *param_3
        injector::WriteMemory<uint8_t> (
            hook::get_pattern ("41 8b 06 39 83 ac 00 00 00 74", 9), 0xeb);
#endif
    }
} _cuts;
