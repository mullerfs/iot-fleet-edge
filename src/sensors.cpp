#include "sensors.h"

#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <Wire.h>

#include "config.h"

static Adafruit_BME280 bme;

static size_t scanI2C(uint8_t* outAddrs, size_t maxAddrs) {
  size_t stored = 0;
  Serial.println("Scanner I2C...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      if (stored < maxAddrs) {
        outAddrs[stored] = addr;
        stored++;
      }
      Serial.print("I2C encontrado: 0x");
      if (addr < 16) {
        Serial.print("0");
      }
      Serial.println(addr, HEX);
    }
  }
  if (stored == 0) {
    Serial.println("Nenhum dispositivo I2C encontrado.");
  }
  return stored;
}

bool sensorsInitBME(uint8_t addr) {
  unsigned status = bme.begin(addr);
  return status != 0;
}

bool sensorsInitBMEAuto() {
  uint8_t i2cAddrs[8];
  size_t i2cCount = scanI2C(i2cAddrs, sizeof(i2cAddrs));
  for (size_t i = 0; i < i2cCount; i++) {
    if (sensorsInitBME(i2cAddrs[i])) {
      Serial.print("BME280 encontrado em 0x");
      if (i2cAddrs[i] < 16) {
        Serial.print("0");
      }
      Serial.println(i2cAddrs[i], HEX);
      return true;
    }
  }
  return false;
}

void sensorsReadBME(float& temperature, float& humidity, float& pressure, float& altitude) {
  temperature = bme.readTemperature();
  humidity    = bme.readHumidity();
  pressure    = bme.readPressure() / 100.0F;
  altitude    = bme.readAltitude(SEALEVELPRESSURE_HPA);
}
