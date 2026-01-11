#include "sensors.h"

#include <Adafruit_BME280.h>
#include <Wire.h>

#include "config.h"

static Adafruit_BME280 bme;

bool sensorsInitBME() {
  unsigned status = bme.begin(BME_I2C_ADDR);
  return status != 0;
}

void sensorsReadBME(float& temperature, float& humidity, float& pressure, float& altitude) {
  temperature = bme.readTemperature();
  humidity    = bme.readHumidity();
  pressure    = bme.readPressure() / 100.0F;
  altitude    = bme.readAltitude(SEALEVELPRESSURE_HPA);
}
