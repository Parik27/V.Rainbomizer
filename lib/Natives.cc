#include "Natives.hh"
#include "Utils.hh"
#include <CTheScripts.hh>

void (*__REQUEST_MODEL) (uint32_t hash);
unsigned short (*__GET_VEHICLE_MODEL_NUMBER_OF_SEATS) (uint32_t hash);

void
InitialiseNatives ()
{
    ConvertCall (
        hook::get_pattern (
            "8b d9 0d ff ff ff 0f c7 ? ? ff ff 00 00 25 ff ff ff 0f 89", -25),
        __REQUEST_MODEL);

    ConvertCall (
        hook::get_pattern (
            "8a 88 9d 00 00 00 80 e1 1f 80 f9 05 75 ? ? 8b 88 b0 00 00 00 ",
            -27),
        __GET_VEHICLE_MODEL_NUMBER_OF_SEATS);
}

/*******************************************************/
void
REQUEST_MODEL (uint32_t hash)
{
    return __REQUEST_MODEL (hash);
}

/*******************************************************/
unsigned short
GET_VEHICLE_MODEL_NUMBER_OF_SEATS (uint32_t vehicleHash)
{
    return __GET_VEHICLE_MODEL_NUMBER_OF_SEATS (vehicleHash);
}
