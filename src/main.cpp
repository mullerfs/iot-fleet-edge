#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <cstring>

#include "config.h"
#include "secrets.h"
#include "sensors.h"
#include "gps.h"
#include "net.h"

HardwareSerial gpsSerial(2);

static unsigned long lastPubMs = 0;
static unsigned long ledLastToggle = 0;
static bool ledState = false;
static uint8_t ledBlinkTarget = 0;
static uint8_t ledBlinkCount = 0;
static bool ledInPause = false;
static const unsigned long ledBlinkIntervalMs = 150;
static unsigned long ledPauseMs = 1000;

static void buildPayload(char* out, size_t outSize) {
  // -------- BME --------
  float temperature = 0.0f, humidity = 0.0f, pressure = 0.0f, altitude = 0.0f;
  sensorsReadBME(temperature, humidity, pressure, altitude);

  // -------- GPS --------
  GpsSnapshot gpsData = gpsGetSnapshot();
  char latStr[32];
  char lngStr[32];
  if (gpsData.hasFix) {
    snprintf(latStr, sizeof(latStr), "%.6f", gpsData.lat);
    snprintf(lngStr, sizeof(lngStr), "%.6f", gpsData.lng);
  } else {
    strcpy(latStr, "null");
    strcpy(lngStr, "null");
  }

  // -------- Network --------
  const IPAddress ip = WiFi.localIP();
  char ipStr[16];
  snprintf(ipStr, sizeof(ipStr), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

  // -------- System --------
  const uint32_t uptime = millis();
  const uint32_t freeHeap = ESP.getFreeHeap();

  int n = snprintf(
    out, outSize,
    "{"
      "\"device_id\":\"%s\","
      "\"system\":{"
        "\"uptime_ms\":%lu,"
        "\"free_heap\":%lu,"
        "\"chip_model\":\"ESP32\","
        "\"chip_revision\":%d,"
        "\"cpu_freq_mhz\":%d"
      "},"
      "\"network\":{"
        "\"wifi\":{"
          "\"ssid\":\"%s\","
          "\"rssi\":%d,"
          "\"local_ip\":\"%s\","
          "\"mac\":\"%s\""
        "},"
        "\"mqtt\":{"
          "\"broker\":\"%s\","
          "\"port\":%d,"
          "\"client_id\":\"%s\","
          "\"connected\":%s"
        "}"
      "},"
      "\"measurements\":{"
        "\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"pressure\":%.2f,"
        "\"altitude_est\":%.2f"
      "},"
      "\"gps\":{"
        "\"fix\":%s,"
        "\"lat\":%s,"
        "\"lng\":%s,"
        "\"alt_m\":%.2f,"
        "\"sats\":%lu,"
        "\"hdop\":%.2f,"
        "\"speed_kmph\":%.2f,"
        "\"course_deg\":%.2f,"
        "\"time\":\"%s\""
      "}"
    "}",
    MQTT_CLIENT_ID,
    uptime,
    freeHeap,
    ESP.getChipRevision(),
    getCpuFrequencyMhz(),
    WiFi.SSID().c_str(),
    WiFi.RSSI(),
    ipStr,
    WiFi.macAddress().c_str(),
    MQTT_HOST,
    MQTT_PORT,
    MQTT_CLIENT_ID,
    netIsMQTTConnected() ? "true" : "false",
    temperature, humidity, pressure, altitude,
    gpsData.hasFix ? "true" : "false",
    latStr, lngStr,
    gpsData.alt,
    (unsigned long)gpsData.sats,
    gpsData.hdop,
    gpsData.speed,
    gpsData.course,
    gpsData.time
  );

  if (n < 0) {
    Serial.println("Erro ao montar payload (snprintf retornou < 0).");
    const char* msg = "{\"error\":\"payload_build_failed\"}";
    strncpy(out, msg, outSize - 1);
    out[outSize - 1] = '\0';
    return;
  }

  if ((size_t)n >= outSize) {
    Serial.println("Payload truncado (buffer pequeno). Enviando aviso minimal.");
    const char* msg = "{\"warn\":\"payload_truncated\"}";
    strncpy(out, msg, outSize - 1);
    out[outSize - 1] = '\0';
  }
}

static void publishTelemetry() {
  char payload[MQTT_BUFFER_SIZE];
  buildPayload(payload, sizeof(payload));

  Serial.print("Publicando em ");
  Serial.print(MQTT_TOPIC);
  Serial.print(": ");
  Serial.println(payload);

  netPublish(MQTT_TOPIC, payload, false);
}

static void setLedPattern(uint8_t target) {
  ledBlinkTarget = target;
  ledBlinkCount = 0;
  ledInPause = false;
  ledState = false;
  ledLastToggle = millis();
  if (target == 1) {
    long pauseCalc = 1000 - (long)(2 * ledBlinkIntervalMs);
    ledPauseMs = pauseCalc > 0 ? (unsigned long)pauseCalc : 0;
  } else {
    ledPauseMs = 1000;
  }
  digitalWrite(LED_PIN, LOW);
}

static void updateLedBlink(uint8_t desired) {
  if (desired != ledBlinkTarget) {
    setLedPattern(desired);
  }
  if (ledBlinkTarget == 0) {
    return;
  }

  unsigned long now = millis();
  if (ledInPause) {
    if (now - ledLastToggle >= ledPauseMs) {
      ledInPause = false;
      ledBlinkCount = 0;
      ledLastToggle = now;
    }
    return;
  }

  if (now - ledLastToggle < ledBlinkIntervalMs) {
    return;
  }

  ledLastToggle = now;
  if (ledState) {
    ledState = false;
    digitalWrite(LED_PIN, LOW);
    ledBlinkCount++;
    if (ledBlinkCount >= ledBlinkTarget) {
      ledInPause = true;
    }
  } else {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  gpsInit(gpsSerial);
  
  if (!sensorsInitBMEAuto()) {
    Serial.println("BME280 nao encontrado. Verifique conexoes/enderecamento.");
     while (true) {
      gpsFeed();
      delay(1000);
    }
  }

  netSetupTLS();
  netConfigureMQTT();

  bool wifiOk = netConnectWiFi(15000, gpsFeed);
  if (wifiOk) {
    netReconnectMQTT(10000, 1000, gpsFeed);
  }
}

void loop() {
  gpsFeed();

  uint8_t desiredBlink = 0;
  if (netIsWiFiConnected()) {
    desiredBlink = 1;
    if (netIsMQTTConnected()) {
      desiredBlink = 2;
      if (gpsGetSnapshot().hasFix) {
        desiredBlink = 3;
      }
    }
  }
  updateLedBlink(desiredBlink);

  static unsigned long lastWiFiAttempt = 0;
  if (!netIsWiFiConnected()) {
    if (millis() - lastWiFiAttempt > 5000) {
      lastWiFiAttempt = millis();
      netConnectWiFi(5000, gpsFeed);
    }
  } else if (!netIsMQTTConnected()) {
    netReconnectMQTT(5000, 500, gpsFeed);
  }

  netLoop();

  const unsigned long now = millis();
  if (netIsWiFiConnected() && netIsMQTTConnected() && (now - lastPubMs >= PUB_INTERVAL_MS)) {
    lastPubMs = now;
    publishTelemetry();
  }
}
