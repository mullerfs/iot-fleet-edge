#include "gps.h"

#include <Arduino.h>

#include "config.h"

static TinyGPSPlus gps;
static HardwareSerial* gpsSerialPtr = nullptr;

void gpsInit(HardwareSerial& serial) {
  gpsSerialPtr = &serial;
  Serial.println("Inicializando GPS...");
  gpsSerialPtr->begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  delay(100);
}

void gpsFeed() {
  if (!gpsSerialPtr) return;
  while (gpsSerialPtr->available() > 0) {
    gps.encode(gpsSerialPtr->read());
  }
}

GpsSnapshot gpsGetSnapshot() {
  GpsSnapshot data{};

  data.ageMs = gps.location.age();
  data.hasFix = gps.location.isValid() && data.ageMs < GPS_FIX_MAX_AGE_MS;
  data.lat = data.hasFix ? gps.location.lat() : 0.0;
  data.lng = data.hasFix ? gps.location.lng() : 0.0;

  data.alt = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
  data.sats = gps.satellites.isValid() ? gps.satellites.value() : 0;
  data.hdop = gps.hdop.isValid() ? gps.hdop.hdop() : 0.0;
  data.speed = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
  data.course = gps.course.isValid() ? gps.course.deg() : 0.0;

  data.time[0] = '\0';
  if (gps.date.isValid() && gps.time.isValid()) {
    snprintf(
      data.time, sizeof(data.time),
      "%04d-%02d-%02dT%02d:%02d:%02dZ",
      gps.date.year(), gps.date.month(), gps.date.day(),
      gps.time.hour(), gps.time.minute(), gps.time.second()
    );
  }

  return data;
}

void gpsDebug() {
  Serial.println("\n=== DEBUG GPS ===");
  GpsSnapshot snap = gpsGetSnapshot();

  Serial.print("Fix: ");
  Serial.print(snap.hasFix ? "SIM" : "NAO");
  Serial.print(" | Age(ms): ");
  Serial.println(snap.ageMs);

  Serial.print("Lat/Lng: ");
  Serial.print(snap.lat, 6);
  Serial.print(", ");
  Serial.println(snap.lng, 6);

  Serial.print("Alt(m): ");
  Serial.println(snap.alt, 2);

  Serial.print("Sats: ");
  Serial.print(snap.sats);
  Serial.print(" | HDOP: ");
  Serial.println(snap.hdop, 2);

  Serial.print("Chars processados: ");
  Serial.println(gps.charsProcessed());
  Serial.print("Sentencas OK: ");
  Serial.println(gps.passedChecksum());
  Serial.print("Sentencas falha: ");
  Serial.println(gps.failedChecksum());

  Serial.print("Bytes no buffer serial: ");
  Serial.println(gpsSerialPtr ? gpsSerialPtr->available() : 0);
}

TinyGPSPlus& gpsInstance() {
  return gps;
}
