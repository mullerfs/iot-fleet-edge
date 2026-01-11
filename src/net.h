#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

bool netConnectWiFi(uint32_t timeoutMs = 15000, void (*pump)() = nullptr);
void netSetupTLS();
void netConfigureMQTT();
bool netReconnectMQTT(uint32_t timeoutMs = 10000, uint32_t retryDelayMs = 1000, void (*pump)() = nullptr);
void netLoop();
bool netIsWiFiConnected();
bool netIsMQTTConnected();
bool netPublish(const char* topic, const char* payload, bool retained = false);
PubSubClient& netClient();
