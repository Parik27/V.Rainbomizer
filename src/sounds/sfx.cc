#include "Utils.hh"
#include "common/config.hh"
#include "memory/GameAddress.hh"
#include <audEngine.hh>
#include <array>

GameVariable<audMetadataMgr *, 100063> audMetadataMgr_GameObjects{};

class SfxRandomizer
{
    /*******************************************************/
    static bool
    ShouldRandomizeGameObject (uint32_t type)
    {
        static constexpr std::array validTypes{
            1,  // Collision
            2,  // Vehicle Trailer
            3,  // Vehicle
            4,  // Vehicle Engine
            8,  // Helicopter
            16, // Boat
            47, // Door
            53, // Train
            56, // Bicycle
            57, // Aeroplane
            87, // Explosion
        };

        return DoesElementExist (validTypes, type);
    }

    /*******************************************************/
    template <typename T>
    static uint8_t *
    audMetadataManager__GetObject_Verifyf24 (audMetadataMgr *mgr, T data,
                                             uint32_t type)
    {
        auto obj = mgr->FindObjectPtr<uint8_t> (data);
        if (obj && *obj != type)
            obj = nullptr;
        return obj;
    }

    /*******************************************************/
    static uint8_t *
    GetRandomGameObjectOfType (audMetadataMgr *mgr, uint32_t type)
    {
        if (!ShouldRandomizeGameObject (type))
            return nullptr;

        std::vector<uint8_t *> validObjects;
        mgr->ForEach<uint8_t> ([&] (uint8_t *data, uint32_t, std::size_t) {
            if (*data == type)
                validObjects.push_back (data);
        });

        if (validObjects.size ())
            return GetRandomElement (validObjects);

        return nullptr;
    }

    /*******************************************************/
    template <typename T>
    static uint8_t *
    RandomizeSfx (audMetadataMgr *mgr, T data, uint32_t type)
    {
        if (mgr == audMetadataMgr_GameObjects)
            {
                uint8_t *obj = GetRandomGameObjectOfType (mgr, type);
                if (obj)
                    return obj;
            }

        return audMetadataManager__GetObject_Verifyf24 (mgr, data, type);
    }

public:
    /*******************************************************/
    SfxRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("SfxRandomizer"))
            return;

        InitialiseAllComponents ();

        // Function that returns the game object metadata for a specific type
        // and hash. Also read the GameObjects metadata manager because the
        // function is also used for other metadata managers.
        MakeJMP64 (GAMEADDR(100064), RandomizeSfx<uint32_t>);
        MakeJMP64 (GAMEADDR(100065), RandomizeSfx<audMetadataRef>);
    }
} sfx;
