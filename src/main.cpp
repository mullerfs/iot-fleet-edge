#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <TinyGPSPlus.h>

// -------------------- Pins --------------------
#define SDA_PIN 18
#define SCL_PIN 19

#define RX2 16
#define TX2 17
#define GPS_BAUD 38400
HardwareSerial gpsSerial(2);

// -------------------- BME --------------------
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

// -------------------- GPS --------------------
TinyGPSPlus gps;

// -------------------- Wi-Fi --------------------
const char* ssid = "三狼之屋";
const char* password = "Casa3lobos";

// -------------------- MQTT/TLS --------------------
const char* mqtt_host = "mqtt.mfs.eng.br";
const int   mqtt_port = 8883;
const char* mqtt_client_id = "device-001";
const char* mqtt_topic     = "sensors/device-001/bme280";

// --- TLS material (cole seus PEMs aqui) ---
static const char* CA_CERT = R"EOF(
-----BEGIN CERTIFICATE-----
MIID8TCCAtmgAwIBAgIUW/TdHMjSwr464n3l54RuuIBsgR0wDQYJKoZIhvcNAQEL
BQAwgYcxCzAJBgNVBAYTAkJSMQswCQYDVQQIDAJNRzETMBEGA1UEBwwKVWJlcmxh
bmRpYTEMMAoGA1UECgwDbWZzMQwwCgYDVQQLDANpb3QxGDAWBgNVBAMMD21xdHQu
bWZzLmVuZy5icjEgMB4GCSqGSIb3DQEJARYRbXVsbGVyQG1mcy5lbmcuYnIwHhcN
MjYwMTExMDIyOTQyWhcNMzYwMTA5MDIyOTQyWjCBhzELMAkGA1UEBhMCQlIxCzAJ
BgNVBAgMAk1HMRMwEQYDVQQHDApVYmVybGFuZGlhMQwwCgYDVQQKDANtZnMxDDAK
BgNVBAsMA2lvdDEYMBYGA1UEAwwPbXF0dC5tZnMuZW5nLmJyMSAwHgYJKoZIhvcN
AQkBFhFtdWxsZXJAbWZzLmVuZy5icjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC
AQoCggEBAP3/Y+pYm5QFTfdvK6LyXrqz/dBIm8X9QU//8XWL5bDTOyIorP4NuVCr
1o8qb9w5kZ7esPv+WhHrdY+GARhg/PspLWpYmnH6TD+Yxna6+NRHJ2yz5Qwm3xV3
gQD2wj2l9ANJmRJWdP50v9opROTfA/L/F2Z6SaHXdEu87UnaQvJ8vH/HwPP7YSV5
sze/58V2eA627cpTXTYQKSaom3GP1aU8dr7trnTjb7Bqg8pRGnL/4/iNWYOBxN01
e3ArbkGlzThy1fsSD4IkGwAR8A52E3e3NrL/GNUZWnn7IfSGAxSupGY0jaJeC8AK
D6UE1vXMSMjXC7Jqgy13BYYiFE16SVMCAwEAAaNTMFEwHQYDVR0OBBYEFD/tSpzc
ixc/n0QJ8aN7sGnPDykBMB8GA1UdIwQYMBaAFD/tSpzcixc/n0QJ8aN7sGnPDykB
MA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAFWMMxRtA/AZEdqX
JN2huRRZ04h9gPbwluBfmzhpxlN0Z1Z21u5TNnfbpGFsdV6W7sleO+JA6gfCp+/d
1CsRUrB6AXDan12lls2eFAd+xjqb61wIlNftcFjRriu1+C0KoTl4BlsvZybHbz36
uG5FTMoMgRa4rFqiAZbQrpdt3r6qmS7hX6VNf+ydICpQ+Hju/0eC8m6MuOdUBn1U
fl/gq09iQaBvrY4vB7FJgCkFGorRqmWEcBQgvJNseJ4RESgz/R9fpQLrQy04fAti
mIKJ9F0VpM6wwuiNr0g8VnasXY4iKh6Pry1XnPIXVh+M60FAABqtvj1BeZadYjer
wENpge8=
-----END CERTIFICATE-----
)EOF";

static const char* CLIENT_CERT = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDbzCCAlcCFBRoYoDK7otDgE9IX01A0h1We3PxMA0GCSqGSIb3DQEBCwUAMIGH
MQswCQYDVQQGEwJCUjELMAkGA1UECAwCTUcxEzARBgNVBAcMClViZXJsYW5kaWEx
DDAKBgNVBAoMA21mczEMMAoGA1UECwwDaW90MRgwFgYDVQQDDA9tcXR0Lm1mcy5l
bmcuYnIxIDAeBgkqhkiG9w0BCQEWEW11bGxlckBtZnMuZW5nLmJyMB4XDTI2MDEx
MTAyNDIwN1oXDTI4MDQxNTAyNDIwN1owYDELMAkGA1UEBhMCQlIxCzAJBgNVBAgM
Ak1HMRMwEQYDVQQHDApVYmVybGFuZGlhMQwwCgYDVQQKDANtZnMxDDAKBgNVBAsM
A2lvdDETMBEGA1UEAwwKZGV2aWNlLTAwMTCCASIwDQYJKoZIhvcNAQEBBQADggEP
ADCCAQoCggEBAKa+kCSoyNbPXDBS1wVC+A6dYmgDiow+qoiOLoAJOQnwmuoyscvK
e26ZW/d9xwzgA0nU/cRnzV8F3qxPeKxTU0LQiO0fPhh4Fyf7yOCg3dNI6pfed9EU
PWJkj8Uidneq20ljsH5xjuBLZTerpw7nGomnXDLACVb2ppnEUQZmKjKFgRHUR2uW
AA9NIljCCvhniFduRQc9hOcF2jMDX5U1Apj6JxVIsPWuVp2Ht2P3kUAlvh9hK7E4
CKoFwJKJ7ShbbWtSe5fRkn6pmd0qMiT9mz1DuYUMHNdfrfp+ks5618iGa2fA9gFl
UHnbbQBLBxbnoZdiB+CgiHQOb1+IkdeGFCcCAwEAATANBgkqhkiG9w0BAQsFAAOC
AQEAyotlG3zE5wd/AOhGI2PHNJitjPF0JBhJ7iVMqUg8zb0/DUyOef5C8EBQXcJG
E4rG40BPQE+6B21o/ACJ5Au/dZvQj7mv2Ck1JBsuXPSp+rJvuNSY7n3uNbJs6XQT
DQyDGEHNjFf7C/hBBL+g4/RfVlpRjZ+pzyATT+Xzc4al9gTewzVnS5rgtU+lgc//
wgtM+m+Mp54G+UI9FVvI8gkApH5GgyLNd461wPMf6V5FlhxZeFarrmzmMFm+bBLL
+Fla1xs+eHDsfT0IEjjvC/zlwOKhAEDY0OSyj50M4u4m4d125oRdodQ+Tfr5ZWDQ
fOqA8TJDKTu6yJ7s98RC1sHwZw==
-----END CERTIFICATE-----
)EOF";

static const char* CLIENT_KEY = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCmvpAkqMjWz1ww
UtcFQvgOnWJoA4qMPqqIji6ACTkJ8JrqMrHLyntumVv3fccM4ANJ1P3EZ81fBd6s
T3isU1NC0IjtHz4YeBcn+8jgoN3TSOqX3nfRFD1iZI/FInZ3qttJY7B+cY7gS2U3
q6cO5xqJp1wywAlW9qaZxFEGZioyhYER1EdrlgAPTSJYwgr4Z4hXbkUHPYTnBdoz
A1+VNQKY+icVSLD1rladh7dj95FAJb4fYSuxOAiqBcCSie0oW21rUnuX0ZJ+qZnd
KjIk/Zs9Q7mFDBzXX636fpLOetfIhmtnwPYBZVB5220ASwcW56GXYgfgoIh0Dm9f
iJHXhhQnAgMBAAECggEAAaeVx0IELtrYlQOCw2LvvTgN7ZmjZVLgiaxc2m5nqilA
uvQ31B2KmLAIO+4mly0d5edUK//FB51c9qQZ6XPbjqfXw//xs3fK8F2iQlW2UyNq
bd608laGFBTgDFpphzR4f+5E+8GQDmURIw/z/2+G/Di0mdhSqhHr2KNxvmODJa8u
B93HC+qYi8dVPTErgXp+veoOeMls4HY6V3PpKw5b1St75aIRenk0DA9/kHXokIn8
Z0W0yCdiVzxl7dzoVvQnkZa0gaHdhD/DLXNaN0ragfcCqhPN8oCEqyszgdsS91My
Z5bJ8oU16TWL0aMHmKnTpes87+DBlsdVQyRTBYCXoQKBgQDRyON8OeRKwGSmVL0P
jEYJXcOVIScnlkrPyd15OJRKnqIG8x7miWLXchbZ1jpBy/EOl6nPQhtJ+w8Q2cCy
y92yCltHhjs/VFIsAwTlGuBCj5Q2JRgbfxydCXa8EN7ACA5sHWualSG7HbZ90qOt
Jt4VcM4rz4Vry/mlTwbXGQhDVwKBgQDLelpTHzPSjYuT0kJg28tVFntt1AUtNDc/
SOasHZGKocfk6NYIJKVOH2q2/iK6viAbjKjGYSNfkjL677K145Q5+IrIVvwWwvc2
Q7I5wnGqO/sbjKTfgH4tRqJGYcsBro8yjGO1+HhspjtcaH9eyD5vg8jtV20zJWwH
8S2OdjaDsQKBgGrO4/uAohnpAJWq+SMsYRAtvAF2gq/mFira9TbdtqqTP51YNPhd
JITaooXAS6LqSc4LK7rJRomRGGeU5kZiA3q9SSIOVqX0sW2VD+3xlBJvdZ9nnXGK
czAE/H3d/Ps6XtVu2TU4DpdtFlfTYeArtNCnwWCO5LgwfudKaAolxY47AoGANspA
bid1vdltXyV7yDcG/mdqXv17U1u/EKRhsLDZktnTSJlFoLW1aV7eTl4KHfy6CpEK
lThs3PwyDA3lMkMQk7eQLzf083AWPx9la1YnYy55hEaP0rr3vttJMOh+UHqfhn31
D3fTfXYZf92Hho8m+MIkpXwoJGCMQWs42CaOsVECgYEAsJ1EbPA6ELLZtuN1NN1O
2yrGkujQTIUrTO7uJQUOJ19bcHW/kxU8icv9tVboihSSpCKEeY5oT+KcfHCm//Jm
1yVWxHtJiIaO+sPkugDvgZRk7eTygS34nbIHca+Pgpb8sw2Ls/su6tOmX/p5KXpx
3wpsf3QDauxYFj2CC0wHdNE=
-----END PRIVATE KEY-----
)EOF";
WiFiClientSecure tlsClient;
PubSubClient mqtt(tlsClient);

// -------------------- Timers --------------------
static unsigned long lastPubMs = 0;
static const unsigned long PUB_INTERVAL_MS = 60000;

// -------------------- Helpers --------------------
static void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi OK, IP: ");
  Serial.println(WiFi.localIP());
}

static void setupTLS() {
  tlsClient.setCACert(CA_CERT);
  tlsClient.setCertificate(CLIENT_CERT);
  tlsClient.setPrivateKey(CLIENT_KEY);

  tlsClient.setHandshakeTimeout(30);
}

static void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando MQTT (mTLS)...");
    bool ok = mqtt.connect(mqtt_client_id);
    if (ok) {
      Serial.println("OK");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(mqtt.state());
      Serial.println(" (tentando novamente em 5s)");
      delay(5000);
    }
  }
}

static void initBME() {
  unsigned status = bme.begin(0x76);
  if (!status) {
    Serial.println("BME280 não encontrado (0x76). Verifique conexões/endereço.");
    Serial.print("SensorID: 0x");
    Serial.println(bme.sensorID(), HEX);
    while (true) delay(1000);
  }
}

static void initGPS() {
  Serial.println("Inicializando GPS...");
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RX2, TX2);
}

// Le o stream do GPS sem travar o loop
static void feedGPS() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
}

static void buildPayload(char* out, size_t outSize) {
  // -------- BME --------
  const float temperature = bme.readTemperature();
  const float humidity    = bme.readHumidity();
  const float pressure    = bme.readPressure() / 100.0F;
  const float altitude    = bme.readAltitude(SEALEVELPRESSURE_HPA);

  // -------- GPS --------
  const bool hasFix = gps.location.isValid() && gps.location.age() < 5000;
  const double lat =  gps.location.lat();
  const double lng = hasFix ? gps.location.lng() : 0.0;

  const double gpsAlt = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
  const uint32_t sats = gps.satellites.isValid() ? gps.satellites.value() : 0;
  const double hdop  = gps.hdop.isValid() ? gps.hdop.hdop() : 0.0;
  const double speed = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
  const double course = gps.course.isValid() ? gps.course.deg() : 0.0;

  char gpsTime[32] = "";
  if (gps.date.isValid() && gps.time.isValid()) {
    snprintf(
      gpsTime, sizeof(gpsTime),
      "%04d-%02d-%02dT%02d:%02d:%02dZ",
      gps.date.year(), gps.date.month(), gps.date.day(),
      gps.time.hour(), gps.time.minute(), gps.time.second()
    );
  }

  // -------- Network --------
  const IPAddress ip = WiFi.localIP();
  char ipStr[16];
  snprintf(ipStr, sizeof(ipStr), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

  // -------- System --------
  const uint32_t uptime = millis();
  const uint32_t freeHeap = ESP.getFreeHeap();

  snprintf(
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
      "\"bme280\":{"
        "\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"pressure\":%.2f,"
        "\"altitude\":%.2f"
      "},"
      "\"gps\":{"
        "\"fix\":%s,"
        "\"lat\":%.6f,"
        "\"lng\":%.6f,"
        "\"alt_m\":%.2f,"
        "\"sats\":%lu,"
        "\"hdop\":%.2f,"
        "\"speed_kmph\":%.2f,"
        "\"course_deg\":%.2f,"
        "\"time\":\"%s\""
      "}"
    "}",
    mqtt_client_id,
    uptime,
    freeHeap,
    ESP.getChipRevision(),
    getCpuFrequencyMhz(),
    WiFi.SSID().c_str(),
    WiFi.RSSI(),
    ipStr,
    WiFi.macAddress().c_str(),
    mqtt_host,
    mqtt_port,
    mqtt_client_id,
    mqtt.connected() ? "true" : "false",
    temperature, humidity, pressure, altitude,
    hasFix ? "true" : "false",
    lat, lng,
    gpsAlt,
    (unsigned long)sats,
    hdop,
    speed,
    course,
    gpsTime
  );
}
static void publishTelemetry() {
  char payload[512];
  buildPayload(payload, sizeof(payload));

  Serial.print("Publicando em ");
  Serial.print(mqtt_topic);
  Serial.print(": ");
  Serial.println(payload);

  // Retained: false para telemetria típica
  mqtt.publish(mqtt_topic, payload, false);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(SDA_PIN, SCL_PIN);

  initGPS();
  initBME();

  connectWiFi();
  setupTLS();

  mqtt.setServer(mqtt_host, mqtt_port);
  mqtt.setKeepAlive(60);

  reconnectMQTT();
}

void loop() {
  // Alimenta o parser do GPS sempre
  feedGPS();

  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  const unsigned long now = millis();
  if (now - lastPubMs >= PUB_INTERVAL_MS) {
    lastPubMs = now;
    publishTelemetry();
  }
}
