#include "net.h"

#include <Arduino.h>

#include "config.h"
#include "secrets.h"

static WiFiClientSecure tlsClient;
static PubSubClient mqtt(tlsClient);

bool netConnectWiFi(uint32_t timeoutMs, void (*pump)()) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Conectando WiFi");
  const unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    delay(250);
    Serial.print(".");
    if (pump) pump();
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi falhou (timeout).");
    return false;
  }

  Serial.println();
  Serial.print("WiFi OK, IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

void netSetupTLS() {
  tlsClient.setCACert(CA_CERT);
  tlsClient.setCertificate(CLIENT_CERT);
  tlsClient.setPrivateKey(CLIENT_KEY);

  tlsClient.setHandshakeTimeout(30);
  tlsClient.setTimeout(15000);
}

void netConfigureMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setKeepAlive(MQTT_KEEPALIVE_SEC);
  mqtt.setBufferSize(MQTT_BUFFER_SIZE);
}

bool netReconnectMQTT(uint32_t timeoutMs, uint32_t retryDelayMs, void (*pump)()) {
  const unsigned long start = millis();

  while (!mqtt.connected() && (millis() - start) < timeoutMs) {
    Serial.print("Conectando MQTT (mTLS)...");
    bool ok = mqtt.connect(MQTT_CLIENT_ID);
    if (ok) {
      Serial.println("OK");
      break;
    }

    Serial.print("falhou, rc=");
    Serial.print(mqtt.state());
    Serial.println(" (nova tentativa em breve)");

    const unsigned long retryStart = millis();
    while ((millis() - retryStart) < retryDelayMs) {
      mqtt.loop();
      if (pump) pump();
      delay(100);
    }
  }

  return mqtt.connected();
}

void netLoop() {
  mqtt.loop();
}

bool netIsWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool netIsMQTTConnected() {
  return mqtt.connected();
}

bool netPublish(const char* topic, const char* payload, bool retained) {
  return mqtt.publish(topic, payload, retained);
}

PubSubClient& netClient() {
  return mqtt;
}
