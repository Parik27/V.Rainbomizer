#include "Natives.hh"
#include "Utils.hh"
#include <CTheScripts.hh>

int (*__CREATE_CAR) (uint32_t hash, Vector3_native *coords, double heading,
                     bool isNetwork, bool thisScriptCheck);
void (*__REQUEST_MODEL) (uint32_t hash);
void (*__LOAD_ALL_OBJECTS_NOW) ();
unsigned short (*__GET_VEHICLE_MODEL_NUMBER_OF_SEATS) (uint32_t hash);

void
InitialiseNatives()
{
    ConvertCall (hook::get_pattern ("8b ec ? 83 ec 50 f3 0f 10 02 f3 0f 10 4a 08",
                                 -17),
              __CREATE_CAR);

    ConvertCall (
        hook::get_pattern (
            "8b d9 0d ff ff ff 0f c7 ? ? ff ff 00 00 25 ff ff ff 0f 89", -25),
        __REQUEST_MODEL);

    ConvertCall (
        hook::get_pattern (
            "? 83 ec 28 ? 8d 0d ? ? ? ? ? 33 c0 b2 01 "),
        __LOAD_ALL_OBJECTS_NOW);

    ConvertCall (
        hook::get_pattern (
            "8a 88 9d 00 00 00 80 e1 1f 80 f9 05 75 ? ? 8b 88 b0 00 00 00 ",
            -27),
        __GET_VEHICLE_MODEL_NUMBER_OF_SEATS);
}

/*******************************************************/
int
CREATE_CAR (uint32_t hash, Vector3_native *coords, double heading,
            bool isNetwork, bool thisScriptCheck)
{
    return __CREATE_CAR (hash, coords, heading, isNetwork, thisScriptCheck);
}

/*******************************************************/
void
REQUEST_MODEL (uint32_t hash)
{
    return __REQUEST_MODEL (hash);
}

/*******************************************************/
void
LOAD_ALL_OBJECTS_NOW ()
{
    return __LOAD_ALL_OBJECTS_NOW ();
}

/*******************************************************/
unsigned short GET_VEHICLE_MODEL_NUMBER_OF_SEATS (uint32_t vehicleHash)
{
    return __GET_VEHICLE_MODEL_NUMBER_OF_SEATS (vehicleHash);
}
