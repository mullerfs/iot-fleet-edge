#pragma once

#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

struct GpsSnapshot {
  bool hasFix;
  double lat;
  double lng;
  double alt;
  uint32_t sats;
  double hdop;
  double speed;
  double course;
  uint32_t ageMs;
  char time[32];
};

void gpsInit(HardwareSerial& serial);
void gpsFeed();
void gpsDebug();
GpsSnapshot gpsGetSnapshot();
TinyGPSPlus& gpsInstance();
