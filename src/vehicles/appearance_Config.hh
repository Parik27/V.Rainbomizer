#pragma once

#include <common/config.hh>

class VehicleAppearanceRandomizerConfig
{
protected:
    RB_C_CONFIG_START
    {
        bool RandomizeCarColours;
        bool RandomizeCarUpgrades;

        int RandomizeLicensePlateOdds = 75;
        int RandomizeHornOdds         = 45;
        int RandomizeArmourOdds       = 45;
    }
    RB_C_CONFIG_END
};
