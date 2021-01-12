#pragma once

#include <cstdint>

uint32_t GetRandomLoadedVehIndex (uint32_t *outNum = nullptr,
                                  bool      trains = false);

void InitialiseDLCDespawnFix ();
