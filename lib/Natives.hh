#pragma once

#include "CMath.hh"
#include <cstdint>

int CREATE_CAR (uint32_t hash, Vector3_native *coords, double heading,
                bool isNetwork, bool thisScriptCheck);
void REQUEST_MODEL (uint32_t hash);
void LOAD_ALL_OBJECTS_NOW ();
unsigned short GET_VEHICLE_MODEL_NUMBER_OF_SEATS (uint32_t vehicleHash);

void InitialiseNatives();
