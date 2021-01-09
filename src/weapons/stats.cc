#include "CItemInfo.hh"
#include "Utils.hh"
#include "common/common.hh"
#include "common/config.hh"
#include "common/parser.hh"
#include <cstdint>
#include <utility>

using CAmmoInfoRandomizer = ParserRandomHelper<
    CAmmoInfo,
    RandomizedFieldsWrapper<ShuffleRandomizer<uint32_t>, "WeaponFlags"_joaat,
                            "AmmoMax50"_joaat, "AmmoMax100"_joaat>>;

using ItemInfoRandomizer = ParserRandomHelperContainer<CAmmoInfoRandomizer>;

class WeaponStatsRandomizer
{
    inline static ItemInfoRandomizer sm_ItemInfoRandomizer;

public:
    /*******************************************************/
    static void
    HandleItemInfoRandomization (bool sample)
    {
        for (CItemInfo *i : CWeaponInfoManager::sm_Instance->aItemInfos)
            {
                std::uint32_t hash;
                sm_ItemInfoRandomizer.AddSample (i, i->GetClassId (hash));
            }
    }

    /*******************************************************/
    WeaponStatsRandomizer ()
    {
        if (!ConfigManager::ReadConfig ("WeaponStatsRandomizer"))
            return;

        // Randomize on game load
        Rainbomizer::Common::AddInitCallback ([] (bool session) {
            static bool toSample = true;
            HandleItemInfoRandomization (std::exchange (toSample, false));
        });

        InitialiseAllComponents ();
    }
} _stats;
