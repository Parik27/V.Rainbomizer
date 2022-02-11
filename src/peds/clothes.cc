#include "CPed.hh"
#include "peds/clothes_Queue.hh"
#include <Utils.hh>
#include <CTheScripts.hh>

#include <common/config.hh>
#include <common/logger.hh>
#include <common/minhook.hh>
#include <common/events.hh>

using namespace NativeLiterals;

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
    static void
    RandomizeComponent (uint32_t ped, int comp)
    {
        
        int drawable = Random<DRAWABLE_NATIVE> (ped, comp);
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
        for (int i = 0; i < 12; i++)
            RandomizeComponent(ped, i);
        
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

        if (!Queue::mData.size())
            return;

        m_CurrentlyRandomizing = true;
        if (RandomizePedClothes (Queue::mData.back ()))
            Queue::mData.pop_back ();
        m_CurrentlyRandomizing = false;
    }

    /*******************************************************/
    template<auto& CPed__GetComponentVariation>
    static void ChangeClothesEvent (CPed* p1, uint32_t p2, uint8_t p3, uint32_t* p4, uint32_t* p5)
    {
        if (!m_CurrentlyRandomizing)
            Queue::Add (p1);
        
        CPed__GetComponentVariation (p1, p2, p3, p4, p5);
    }
    
public:
    ClothesRandomizer ()
    {        
        RB_C_DO_CONFIG_NO_OPTIONS ("ClothesRandomizer");

        Rainbomizer::Events ().OnRunThread += ProcessUpgradesQueue;
        Rainbomizer::Events ().OnFade +=
            [] { Queue::Add (int ("PLAYER_PED_ID"_n())); };

        "SET_PED_COMPONENT_VARIATION"_n.Hook ([] (scrThread::Info *info) {});
        "SET_PED_PROP_INDEX"_n.Hook ([] (scrThread::Info *info) {});
        
        REGISTER_HOOK ("89 44 ? ? e8 ? ? ? ? ? 8b ? ? ? 8b ? ? ? 8d ? ? ? 81 c2", 4,
                       ChangeClothesEvent, void, CPed*, uint32_t, uint8_t, uint32_t*, uint32_t*);
    }
} g_ClothesRandomizer;
