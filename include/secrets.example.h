#pragma once
// Preencha estes valores e copie para include/secrets.h (arquivo ignorado pelo Git).
// Nunca faça commit de chaves/senhas/certificados.

// Wi-Fi
static const char* WIFI_SSID = "YOUR_WIFI_SSID";
static const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// MQTT
static const char* MQTT_HOST      = "mqtt.example.com";
static const int   MQTT_PORT      = 8883;
static const char* MQTT_CLIENT_ID = "device-001";
static const char* MQTT_TOPIC     = "sensors/device-001/bme280";

// TLS PEMs
static const char* CA_CERT = R"EOF(
-----BEGIN CERTIFICATE-----
... your CA cert ...
-----END CERTIFICATE-----
)EOF";

static const char* CLIENT_CERT = R"EOF(
-----BEGIN CERTIFICATE-----
... your device cert ...
-----END CERTIFICATE-----
)EOF";

static const char* CLIENT_KEY = R"EOF(
-----BEGIN PRIVATE KEY-----
... your device key ...
-----END PRIVATE KEY-----
)EOF";
