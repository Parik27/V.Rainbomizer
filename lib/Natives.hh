#pragma once

#include "CMath.hh"
#include "memory/GameAddress.hh"
#include <cstdint>

static GameFunction<100149, void (uint32_t)> REQUEST_MODEL{};
static GameFunction<100150, unsigned short(uint32_t)> GET_VEHICLE_MODEL_NUMBER_OF_SEATS{};

void InitialiseNatives ();
