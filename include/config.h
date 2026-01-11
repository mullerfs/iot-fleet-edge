#pragma once

// -------------------- Pins --------------------
constexpr int SDA_PIN = 18;
constexpr int SCL_PIN = 19;

constexpr int GPS_RX_PIN = 16;
constexpr int GPS_TX_PIN = 17;
constexpr uint32_t GPS_BAUD = 9600;

// -------------------- Sensors --------------------
constexpr uint8_t BME_I2C_ADDR = 0x76;
constexpr float SEALEVELPRESSURE_HPA = 1013.25f;
constexpr uint32_t GPS_FIX_MAX_AGE_MS = 5000;

// -------------------- Timers/Networking --------------------
constexpr unsigned long PUB_INTERVAL_MS = 60000;
constexpr int MQTT_BUFFER_SIZE = 1024;
constexpr uint16_t MQTT_KEEPALIVE_SEC = 60;
