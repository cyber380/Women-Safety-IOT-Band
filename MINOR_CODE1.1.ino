/********************
 * ESP32 Emergency Alert + Heart & SpO2 Logger
 * GPS + Telegram + Button + MAX30102 + Real Time
 ********************/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "time.h"

/* ================== PIN DEFINITIONS ================== */
#define BUTTON_PIN 18
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define SDA_PIN 21
#define SCL_PIN 22

/* ================== WiFi ================== */
const char* WIFI_SSID = "hi";
const char* WIFI_PASSWORD = "9437265232";

/* ================== Telegram ================== */
#define TELEGRAM_BOT_TOKEN "7704205587:AAFuQbEns3q8tBlkvMc-FqFFe3GgR-qHo2I"
#define CHAT_ID_1 "1995560946"
#define CHAT_ID_2 "1999875456"

/* ================== Time ================== */
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;   // India
const int daylightOffset_sec = 0;

/* ================== Objects ================== */
WiFiClientSecure secureClient;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, secureClient);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
MAX30105 particleSensor;

/* ================== HR & SpO2 ================== */
uint32_t irBuffer[100];
uint32_t redBuffer[100];
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

unsigned long lastHRTime = 0;
const unsigned long HR_INTERVAL = 3000;

/* ================== Stabilization ================== */
#define HR_MIN 40
#define HR_MAX 180

int hrHistory[5] = {0,0,0,0,0};
int hrIndex = 0;
int stableHR = 0;
int stableSpO2 = 0;

/* ================== Flags ================== */
bool prevButton = HIGH;
bool gpsReady = false;

/* ================== SETUP ================== */
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected");

  secureClient.setInsecure();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) delay(500);
  Serial.println("Time synced");

  waitForGPS();

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x3F);
  particleSensor.setPulseAmplitudeIR(0x3F);

  Serial.println("System Ready");
}

/* ================== LOOP ================== */
void loop() {
  while (gpsSerial.available()) gps.encode(gpsSerial.read());

  if (millis() - lastHRTime > HR_INTERVAL) {
    readVitals();
    lastHRTime = millis();
  }

  bool btn = digitalRead(BUTTON_PIN);
  if (prevButton == HIGH && btn == LOW) {
    sendLocation();
    delay(800);
  }
  prevButton = btn;
}

/* ================== GPS WAIT ================== */
void waitForGPS() {
  Serial.println("Waiting for GPS...");
  while (!gps.location.isValid()) {
    while (gpsSerial.available()) gps.encode(gpsSerial.read());
  }
  gpsReady = true;
}

/* ================== HEART + SPO2 ================== */
void readVitals() {

  for (byte i = 0; i < 100; i++) {
    while (!particleSensor.available()) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i]  = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(
    irBuffer, 100, redBuffer,
    &spo2, &validSPO2,
    &heartRate, &validHeartRate
  );

  bool finger = irBuffer[99] > 30000;

  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char t[10];
  strftime(t, sizeof(t), "%H:%M:%S", &timeinfo);

  if (finger && validHeartRate && validSPO2 &&
      heartRate >= HR_MIN && heartRate <= HR_MAX &&
      spo2 >= 80 && spo2 <= 100) {

    hrHistory[hrIndex] = heartRate;
    hrIndex = (hrIndex + 1) % 5;

    int sum = 0;
    int count = 0;
    for (int i = 0; i < 5; i++) {
      if (hrHistory[i] > 0) {
        sum += hrHistory[i];
        count++;
      }
    }
    if (count > 0) stableHR = sum / count;
    stableSpO2 = spo2;
  }

  Serial.print(t);
  Serial.print(",");

  if (stableHR > 0 && stableSpO2 > 0) {
    Serial.print(stableHR);
    Serial.print(",");
    Serial.print(stableSpO2);
    Serial.println(",1");
  } else {
    Serial.println("0,0,0");
  }
}

/* ================== TELEGRAM ================== */
void sendLocation() {
  String msg = "🚨 EMERGENCY\n\n";
  msg += "Lat: " + String(gps.location.lat(), 6) + "\n";
  msg += "Lon: " + String(gps.location.lng(), 6) + "\n";
  msg += "https://www.google.com/maps?q=";
  msg += String(gps.location.lat(), 6) + ",";
  msg += String(gps.location.lng(), 6);

  bot.sendMessage(CHAT_ID_1, msg, "");
  bot.sendMessage(CHAT_ID_2, msg, "");
}