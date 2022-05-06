#include "CPed.hh"
#include "common/common.hh"
#include "peds/clothes_Queue.hh"
#include "rage.hh"
#include <Utils.hh>
#include <CTheScripts.hh>

#include <array>

#include <common/config.hh>
#include <common/logger.hh>
#include <common/minhook.hh>
#include <common/events.hh>

#ifdef ENABLE_DEBUG_MENU
#include <debug/base.hh>
#endif

using namespace NativeLiterals;

enum eComponentType : int32_t
{
    CPT_FACE,
    CPT_MASK,
    CPT_HAIR,
    CPT_TORSO,
    CPT_LEG,
    CPT_PARACHUTE,
    CPT_SHOES,
    CPT_ACCESSORY,
    CPT_UNDERSHIRT,
    CPT_KEVLAR,
    CPT_BADGE,
    CPT_TORSO2,
    CPT_NUM_COMPONENTS
};

class ClothesRandomizer
{
    using Queue = ClothesRandomizer_Queue;

    inline static bool m_CurrentlyRandomizing = false;
    
    static constexpr uint32_t DRAWABLE_NATIVE
        = "GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS"_joaat;
    static constexpr uint32_t TEXTURE_NATIVE
        = "GET_NUMBER_OF_PED_TEXTURE_VARIATIONS"_joaat;
    static constexpr uint32_t PROP_DRAW_NATIVE
        = "GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS"_joaat;
    static constexpr uint32_t PROP_TEX_NATIVE
        = "GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS"_joaat;

    struct NSFWComponentData
    {
        uint32_t       modelHash;
        eComponentType component;
        uint32_t       variation;
    };

    inline static std::vector<NSFWComponentData> sm_NsfwComponents;

    RB_C_CONFIG_START
    {
        int RandomizeOdds = 80;

        int MaskOdds      = 20;
        int ParachuteOdds = 20;

        int  ForcedRandomComponent = -1;
        bool EnableNSFWComponents  = false;
    }
    RB_C_CONFIG_END

    inline static std::array<int, CPT_NUM_COMPONENTS> m_ComponentOdds
        = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    /*******************************************************/
    template <uint32_t NativeHash, typename... Args>
    static uint32_t
    Random (Args... args)
    {
        uint32_t max
            = NativeManager::InvokeNative<uint32_t> (NativeHash, args...);

        return max == 0 ? 0 : RandomInt (max - 1);
    }

    /*******************************************************/
    static bool
    IsVariationAllowed (int comp, uint32_t drawable, uint32_t ped)
    {
        if (Config ().EnableNSFWComponents)
            return true;

        ReadNSFWComponentsData ();
        
        uint32_t modelHash = "GET_ENTITY_MODEL"_n(ped);
        for (auto &i : sm_NsfwComponents)
            {
                if (i.modelHash == modelHash && i.component == comp
                    && i.variation == drawable)
                    return false;
            }

        return true;
    }

    /*******************************************************/
    static void
    RandomizeComponent (uint32_t ped, int comp)
    {
        int drawable = Random<DRAWABLE_NATIVE> (ped, comp);

        // NSFW check
        if (!IsVariationAllowed(comp, drawable, ped))
            return;

        int texture  = Random<TEXTURE_NATIVE> (ped, comp, drawable);

        "SET_PED_COMPONENT_VARIATION"_n(ped, comp, drawable, texture,
                                        RandomInt (3));

        if (comp >= 4)
            return;

        int propDrawable = Random<PROP_DRAW_NATIVE> (ped, comp);
        int propTexture  = Random<PROP_TEX_NATIVE> (ped, comp, propDrawable);

        "SET_PED_PROP_INDEX"_n(ped, comp, propDrawable, propTexture, 1);
    }

    /*******************************************************/
    static bool
    RandomizePedClothes (uint32_t ped)
    {
        if (!RandomBool (Config ().RandomizeOdds))
            return true;

        m_ComponentOdds[CPT_FACE]      = Config ().MaskOdds;
        m_ComponentOdds[CPT_MASK]      = Config ().MaskOdds;
        m_ComponentOdds[CPT_PARACHUTE] = Config ().ParachuteOdds;
        m_ComponentOdds[CPT_KEVLAR]    = Config ().ParachuteOdds;
        m_ComponentOdds[CPT_ACCESSORY] = Config ().ParachuteOdds;
        m_ComponentOdds[CPT_TORSO2]    = 0;

        if (Config ().ForcedRandomComponent != -1)
            {
                RandomizeComponent (ped, Config ().ForcedRandomComponent);
                return true;
            }

        for (int i = 0; i < CPT_NUM_COMPONENTS; i++)
            {
                if (m_ComponentOdds[i] == -1 || RandomBool (m_ComponentOdds[i]))
                    RandomizeComponent (ped, i);
            }

        return true;
    }

    /*******************************************************/
    static void
    ProcessUpgradesQueue (uint64_t *, uint64_t *, scrProgram *,
                          scrThreadContext *ctx)
    {
        if (ctx->m_nScriptHash != "main"_joaat)
            return;

        std::lock_guard guard (Queue::mMutex);

        if (!Queue::mData.size ())
            return;

        m_CurrentlyRandomizing = true;
        if (RandomizePedClothes (Queue::mData.back ()))
            Queue::mData.pop_back ();
        m_CurrentlyRandomizing = false;
    }

    /*******************************************************/
    template <auto &CPed__GetComponentVariation>
    static void
    ChangeClothesEvent (CPed *p1, uint32_t p2, uint8_t p3, uint32_t *p4,
                        uint32_t *p5)
    {
        if (!m_CurrentlyRandomizing)
            Queue::Add (p1);

        CPed__GetComponentVariation (p1, p2, p3, p4, p5);
    }

    /*******************************************************/
    static void
    ReadNSFWComponentsData ()
    {
        static bool sm_Initialised = false;
        if (std::exchange (sm_Initialised, true))
            return;

        FILE *f = Rainbomizer::Common::GetRainbomizerDataFile (
            "NSFW_Components.txt");

        if (!f)
            return;

        char line[512] = {0};
        while (fgets(line, 512, f))
            {
                char model[64] = {0};
                NSFWComponentData data;

                if (sscanf (line, "%s %d %d", model, &data.component,
                            &data.variation)
                    != 3)
                    continue;

                data.modelHash = rage::atStringHash (model);
                sm_NsfwComponents.push_back (data);
            }
    }

public:
    ClothesRandomizer ()
    {
        RB_C_DO_CONFIG ("ClothesRandomizer", RandomizeOdds, MaskOdds,
                        ParachuteOdds, ForcedRandomComponent,
                        EnableNSFWComponents);

        m_ComponentOdds[CPT_FACE]      = Config ().MaskOdds;
        m_ComponentOdds[CPT_MASK]      = Config ().MaskOdds;
        m_ComponentOdds[CPT_PARACHUTE] = Config ().ParachuteOdds;
        m_ComponentOdds[CPT_KEVLAR]    = Config ().ParachuteOdds;
        m_ComponentOdds[CPT_ACCESSORY] = Config ().ParachuteOdds;
        m_ComponentOdds[CPT_TORSO2]    = 0;

        Rainbomizer::Events ().OnRunThread += ProcessUpgradesQueue;
        Rainbomizer::Events ().OnFade +=
            [] { Queue::Add (int ("PLAYER_PED_ID"_n())); };

#ifdef ENABLE_DEBUG_MENU
        DebugInterfaceManager::AddAction ("Randomize Player Clothes",
                                          [] (bool) {
                                              Queue::Add (
                                                  int ("PLAYER_PED_ID"_n()));
                                          });
#endif

        "SET_PED_COMPONENT_VARIATION"_n.Hook ([] (scrThread::Info *info) {});
        "SET_PED_PROP_INDEX"_n.Hook ([] (scrThread::Info *info) {});
        
        REGISTER_HOOK ("89 44 ? ? e8 ? ? ? ? ? 8b ? ? ? 8b ? ? ? 8d ? ? ? 81 c2", 4,
                       ChangeClothesEvent, void, CPed*, uint32_t, uint8_t, uint32_t*, uint32_t*);
    }
} g_ClothesRandomizer;
