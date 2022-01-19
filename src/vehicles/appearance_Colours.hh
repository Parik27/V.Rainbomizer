#pragma once

#include <map>
#include <array>

#include <Utils.hh>

#include <exceptions/exceptions_Vars.hh>

#include <HSL.hh>
#include <CARGB.hh>
#include <CVehicle.hh>

/* Helper class for VehicleAppearanceRandomizer
   It's intended to be used as such:

   1. Restore original colours of vehicle
   2. Call function to update vehicle colours (to check if the vehicle was
       resprayed, etc.)
   3. Call StoreVehicleColourData to store new vehicle colours
   4. If the last call returned true, randomize vehicle colours (since the
       colorus changed)
   5. Finally, set the random colours of the vehicle
 */
class VehicleColourRandomizer
{
    struct VehicleColourData
    {
        CARGB OriginalColours[4];
        CARGB RandomColours[4];
    };

    inline static std::map<CVehicle *, VehicleColourData> mColourData;

public:
    /* Function to restore the original colours of the vehicle */
    static void
    RestoreVehicleColourData (CCustomShaderEffectVehicle *shader, CVehicle *veh)
    {
        if (auto *data = LookupMap (mColourData, veh))
            {
                CARGB *colours = shader->GetColours ();

                for (int i = 0; i < 4; i++)
                    colours[i] = data->OriginalColours[i];
            }
    }

    /*******************************************************/
    /* Function to store the current colours of the vehicle. Returns true if the
     * colours changed */
    static bool
    StoreVehicleColourData (CCustomShaderEffectVehicle *shader, CVehicle *veh)
    {
        auto  &data    = mColourData[veh];
        CARGB *colours = shader->GetColours ();

        bool changed = false;
        for (int i = 0; i < 4; i++)
            if (std::exchange (data.OriginalColours[i], colours[i])
                != colours[i])
                changed = true;

        return changed;
    }

    /*******************************************************/
    /* Randomizes the colours stored in the memory */
    static void
    RandomizeVehicleColours (CVehicle *veh)
    {
        using Rainbomizer::HSL;

        for (int i = 0; i < 4; i++)
            mColourData[veh].RandomColours[i]
                = HSL (RandomFloat (360.0f), 1.0f, RandomFloat (1.0f));
    }

    /*******************************************************/
    /* Changes the vehicle shader colours to stored random colours */
    static void
    StoreRandomizedVehColours (CVehicle *veh, CARGB *colours)
    {
        for (int i = 0; i < 4; i++)
            colours[i] = mColourData[veh].RandomColours[i];
    }
};
