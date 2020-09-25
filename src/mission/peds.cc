#include "Utils.hh"
#include "CPed.hh"
#include "CStreaming.hh"
#include "CTheScripts.hh"

#include <mutex>
#include <set>

#include "common/common.hh"
#include "common/config.hh"
#include "common/logger.hh"

class CPedFactory;

CPed *(*CPedFactory_CreateNonCopPed_5c6) (CPedFactory *, uint8_t *, uint32_t,
                                          uint64_t, uint8_t);

class PedRandomizer
{
    inline static std::mutex CreatePedMutex;

    static auto &
    Config ()
    {
        static struct Config
        {
            uint32_t ForcedPedHash = -1;
        } m_Config;

        return m_Config;
    }

    /*******************************************************/
    /* RAII wrapper to swap two models */
    /*******************************************************/
    class ModelSwapper
    {
        uint32_t m_IdA;
        uint32_t m_IdB;

        static void
        SwapModels (uint32_t a, uint32_t b)
        {

            /*
              Most Important:

              CombatInfo
              NavCapabilities
              TaskDataInfo

              Others:
              RelationshipGroup?
              PedType?
              DecisionMaker??
              DefaultUnarmedWeapon??
              PedPersonality???
              PedCapsule???

              Decided Against:

              PerceptionInfo
              BrawlingStyle
              PedComponentCloth
              PedComponentSet
              DefaultTaskData
             */

            auto modelA = CStreaming::GetModelByIndex<CPedModelInfo> (a);
            auto modelB = CStreaming::GetModelByIndex<CPedModelInfo> (b);

#define SWAP_FIELD(field) std::swap (modelA->field, modelB->field);

            SWAP_FIELD (m_nCombatInfo);
            SWAP_FIELD (m_nPedType);
            SWAP_FIELD (GetInitInfo ().m_nTaskDataName);
            SWAP_FIELD (GetInitInfo ().m_nRelationshipGroup);
            SWAP_FIELD (GetInitInfo ().m_nDecisionMakerName);
            SWAP_FIELD (GetInitInfo ().m_nNavCapabilitiesName);

#undef SWAP_FIELD
        }

    public:
        ModelSwapper (uint32_t idA, uint32_t idB) : m_IdA (idA), m_IdB (idB)
        {
            SwapModels (idA, idB);
        }

        ~ModelSwapper () { SwapModels (m_IdA, m_IdB); }
    };

    /*******************************************************/
    static bool
    IsPlayerModel (CBaseModelInfo *model)
    {
        switch (model->m_nHash)
            {
            case "player_zero"_joaat:
            case "player_one"_joaat:
            case "player_two"_joaat: return true;
            }

        return false;
    }

    /*******************************************************/
    static void
    ProcessPedStreaming ()
    {
        const int       TIMEOUT   = 5;
        static uint32_t timestamp = time (NULL);

        if (time (NULL) - timestamp > TIMEOUT)
            {
                auto PedHashes = Rainbomizer::Common::GetPedHashes ();
                CStreaming::RequestModel (CStreaming::GetModelIndex (
                                              GetRandomElement (PedHashes)),
                                          0);

                timestamp = time (NULL);
            }
    }

    /*******************************************************/
    static uint32_t
    GetRandomPedModel (uint32_t model)
    {
        if (IsPlayerModel (CStreaming::GetModelByIndex (model)))
            return model;

        // Forced Ped
        if (Config ().ForcedPedHash != -1)
            {
                uint32_t id
                    = CStreaming::GetModelIndex (Config ().ForcedPedHash);

                if (CStreaming::HasModelLoaded (id))
                    return id;

                CStreaming::RequestModel (id, 7);
                return model;
            }

        // Random Ped
        std::set<uint32_t> peds;
        auto               groups = CStreaming::sm_Instance;

        groups->mAppropriatePedsSet.for_each (
            [&peds] (int val) { peds.insert (val); });
        groups->mInAppropriatePedsSet.for_each (
            [&peds] (int val) { peds.insert (val); });
        groups->mCopsSet.for_each ([&peds] (int val) { peds.insert (val); });

        if (peds.size () < 1)
            return model;

        return GetRandomElement (peds);
    }

    /*******************************************************/
    template <auto& OrigFunc>
    static CPed *
    RandomizePed (CPedFactory *fac, uint8_t *p2, uint32_t model, uint64_t p4,
                  uint8_t p5)
    {
        const std::lock_guard g (CreatePedMutex);

        ProcessPedStreaming ();

        uint32_t           newModel = GetRandomPedModel (model);
        const ModelSwapper swap (model, newModel);

        
        return OrigFunc (fac, p2, newModel, p4, p5);
    }

public:
    /*******************************************************/
    PedRandomizer ()
    {

        std::string ForcedPed;
        if (!ConfigManager::ReadConfig ("PedRandomizer",
                                        std::pair ("ForcedPed", &ForcedPed)))
            return;

        
        if (ForcedPed.size())
            Config ().ForcedPedHash = rage::atStringHash (ForcedPed.c_str ());
        
        InitialiseAllComponents ();

        // Hooks
        RegisterHook ("8b c0 ? 8b ? ? 8b ? ? 88 7c ? ? e8 ? ? ? ? eb ?", 13,
                      CPedFactory_CreateNonCopPed_5c6,
                      RandomizePed<CPedFactory_CreateNonCopPed_5c6>);

        // This patch changes the value of TB_DEAD from 0 to 1 to prevent
        // aquatic animals from dying. This effectively changes all TB_DEAD
        // entries to TB_COLD.

        // *hook::get_pattern<uint32_t> (
        //     "6d 99 17 b8 00 00 00 00 7f 9a 20 e4 01 00 00 00", 4)
        //     = 1;
    }
} peds;
