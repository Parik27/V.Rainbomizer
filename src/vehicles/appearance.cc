#include <map>
#include <array>

#include <Utils.hh>

#include <mutex>

#include <common/parser.hh>
#include <common/events.hh>
#include <common/config.hh>
#include <common/logger.hh>
#include <common/minhook.hh>

#include <exceptions/exceptions_Vars.hh>

#include <HSL.hh>
#include <CARGB.hh>
#include <CPools.hh>
#include <CVehicle.hh>
#include <CTheScripts.hh>

#include "appearance_Colours.hh"
#include "appearance_Upgrades.hh"
#include "appearance_Config.hh"

uint32_t (*CCustomShaderEffectVehicle_SetForVehicle_134) (
    CCustomShaderEffectVehicle *, CVehicle *);

class VehicleAppearanceRandomizer : VehicleAppearanceRandomizerConfig
{

    /*******************************************************/
    static uint32_t
    RandomizeVehicleAppearance (CCustomShaderEffectVehicle *shader,
                                CVehicle                   *veh)
    {
        using VehColRandomizer = VehicleColourRandomizer;
        using VehUpgRandomizer = VehicleUpgradesRandomizer;

        VehColRandomizer::RestoreVehicleColourData (shader, veh);

        uint32_t ret
            = CCustomShaderEffectVehicle_SetForVehicle_134 (shader, veh);

        CARGB *colours = shader->GetColours ();
        if (VehColRandomizer::StoreVehicleColourData (shader, veh))
            {
                if (Config ().RandomizeCarUpgrades)
                    VehUpgRandomizer::EnqueueUpgradesRandomization (veh);

                if (Config ().RandomizeCarColours)
                    VehColRandomizer::RandomizeVehicleColours (veh);
            }

        if (Config ().RandomizeCarColours)
            VehColRandomizer::StoreRandomizedVehColours (veh, colours);

        return ret;
    }

public:
    /*******************************************************/
    VehicleAppearanceRandomizer ()
    {
        RB_C_DO_CONFIG ("VehicleAppearanceRandomizer", RandomizeCarColours,
                        RandomizeCarUpgrades, RandomizeHornOdds,
                        RandomizeArmourOdds, RandomizeLicensePlateOdds);

        MinHookWrapper::HookBranchDestination (
            "85 c9 74 ? ? 8b d3 e8 ? ? ? ? 84 c0 74 ? ? 84 ff 74", 7,
            CCustomShaderEffectVehicle_SetForVehicle_134,
            RandomizeVehicleAppearance);

        if (Config ().RandomizeCarUpgrades)
            VehicleUpgradesRandomizer::Init ();
    }

} g_VehicleAppearanceRandomizer;
