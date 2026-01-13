#pragma once

#include <stdint.h>

bool sensorsInitBME(uint8_t addr);
bool sensorsInitBMEAuto();
void sensorsReadBME(float& temperature, float& humidity, float& pressure, float& altitude);
