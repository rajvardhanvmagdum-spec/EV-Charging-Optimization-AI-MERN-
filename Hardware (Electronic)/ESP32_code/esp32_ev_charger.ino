/*
  EV Charging Station - ESP32 Firmware
  ------------------------------------
  Reads: RFID (auth), ACS712 current x2, Voltage sensor x2, DS18B20 temp
  Controls: 2-channel relay, OLED display, Green/Red LEDs, Buzzer
  Sends: JSON telemetry over WiFi to the Node.js/Express backend (MongoDB)

  Required libraries (install via Arduino Library Manager):
    - MFRC522           (RFID)
    - Adafruit GFX       (OLED)
    - Adafruit SSD1306    (OLED)
    - OneWire            (DS18B20)
    - DallasTemperature   (DS18B20)
    - ArduinoJson         (JSON payload)
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ---------- WIFI + SERVER CONFIG ----------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL    = "http://192.168.1.100:5000/api/sessions/telemetry"; // your PC/server IP
const char* STATION_ID    = "STATION_01"; // matches a MongoDB ChargingStation _id or name

// ---------- PIN MAP ----------
#define RFID_SS_PIN   5
#define RFID_RST_PIN  16
// RFID SCK=18, MOSI=23, MISO=19 (hardware SPI, no need to define)

#define OLED_SDA      21
#define OLED_SCL      22
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

#define RELAY1_PIN    25
#define RELAY2_PIN    26

#define ACS712_1_PIN  34
#define ACS712_2_PIN  35
#define VOLT1_PIN     32
#define VOLT2_PIN     33

#define GREEN1_PIN    27
#define GREEN2_PIN    14
#define RED1_PIN      12
#define RED2_PIN      13
#define BUZZER_PIN    4

#define TEMP_PIN      15

// ---------- CALIBRATION (tune these for your exact sensor boards) ----------
const float ADC_MAX_VOLTAGE   = 3.3;
const int   ADC_RESOLUTION    = 4095;
const float ACS712_SENSITIVITY = 0.100; // V per A -> 0.185 (5A) / 0.100 (20A) / 0.066 (30A) module
const float ACS712_ZERO_V      = 1.65;  // measured zero-current output after your divider
const float VOLTAGE_CALIBRATION = 7.50; // multiply divider reading to get real pack voltage

const float OVERHEAT_TEMP_C   = 55.0;
const unsigned long SEND_INTERVAL_MS = 5000; // telemetry push interval

// Simple local allowlist demo - replace with a backend lookup call for production
String allowedUIDs[] = {"A1B2C3D4", "11223344"};
const int allowedCount = 2;

// ---------- OBJECTS ----------
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

// ---------- STATE ----------
bool port1Active = false;
bool port2Active = false;
String port1UID = "";
String port2UID = "";
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);

  SPI.begin();
  rfid.PCD_Init();

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  showMessage("Booting...");

  tempSensor.begin();

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  pinMode(GREEN1_PIN, OUTPUT);
  pinMode(GREEN2_PIN, OUTPUT);
  pinMode(RED1_PIN, OUTPUT);
  pinMode(RED2_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  connectWiFi();
  showMessage("Tap RFID card");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  handleRFID();
  monitorPort(1, RELAY1_PIN, ACS712_1_PIN, VOLT1_PIN, GREEN1_PIN, RED1_PIN, port1Active, port1UID);
  monitorPort(2, RELAY2_PIN, ACS712_2_PIN, VOLT2_PIN, GREEN2_PIN, RED2_PIN, port2Active, port2UID);

  if (millis() - lastSend > SEND_INTERVAL_MS) {
    sendTelemetry(1, ACS712_1_PIN, VOLT1_PIN, port1Active, port1UID);
    sendTelemetry(2, ACS712_2_PIN, VOLT2_PIN, port2Active, port2UID);
    lastSend = millis();
  }
}

// ---------- RFID ----------
void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  rfid.PICC_HaltA();

  if (!isAllowed(uid)) {
    beep(3, 100);
    showMessage("Access denied");
    return;
  }

  // assign to whichever port is free
  if (!port1Active) {
    port1Active = true;
    port1UID = uid;
    digitalWrite(RELAY1_PIN, HIGH);
    beep(1, 150);
    showMessage("Port 1 charging\nUID: " + uid);
  } else if (!port2Active) {
    port2Active = true;
    port2UID = uid;
    digitalWrite(RELAY2_PIN, HIGH);
    beep(1, 150);
    showMessage("Port 2 charging\nUID: " + uid);
  } else {
    beep(2, 100);
    showMessage("All ports busy");
  }
}

bool isAllowed(String uid) {
  for (int i = 0; i < allowedCount; i++) {
    if (allowedUIDs[i] == uid) return true;
  }
  return false;
}

// ---------- PORT MONITORING ----------
void monitorPort(int portNum, int relayPin, int currentPin, int voltPin,
                  int greenPin, int redPin, bool &active, String &uid) {
  if (!active) {
    digitalWrite(greenPin, LOW);
    return;
  }

  float current = readCurrent(currentPin);
  float temperature = tempSensor.getTempCByIndex(0);

  bool fault = (temperature > OVERHEAT_TEMP_C);

  if (fault) {
    digitalWrite(relayPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    beep(4, 80);
    showMessage("Port " + String(portNum) + " OVERHEAT\nStopped");
    active = false;
    uid = "";
    return;
  }

  // charge complete when current drops near zero after being active
  if (current < 0.15) {
    digitalWrite(relayPin, LOW);
    digitalWrite(greenPin, LOW);
    beep(2, 120);
    showMessage("Port " + String(portNum) + " complete");
    active = false;
    uid = "";
    return;
  }

  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, LOW);
}

float readCurrent(int pin) {
  int raw = analogRead(pin);
  float voltage = (raw / (float)ADC_RESOLUTION) * ADC_MAX_VOLTAGE;
  return (voltage - ACS712_ZERO_V) / ACS712_SENSITIVITY;
}

float readVoltage(int pin) {
  int raw = analogRead(pin);
  float voltage = (raw / (float)ADC_RESOLUTION) * ADC_MAX_VOLTAGE;
  return voltage * VOLTAGE_CALIBRATION;
}

// ---------- NETWORKING ----------
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  showMessage("Connecting WiFi...");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    tries++;
  }
}

void sendTelemetry(int portNum, int currentPin, int voltPin, bool active, String uid) {
  if (WiFi.status() != WL_CONNECTED) return;

  StaticJsonDocument<256> doc;
  doc["stationId"] = STATION_ID;
  doc["portId"] = portNum;
  doc["uid"] = uid;
  doc["current"] = readCurrent(currentPin);
  doc["voltage"] = readVoltage(voltPin);
  doc["temperature"] = tempSensor.getTempCByIndex(0);
  doc["status"] = active ? "charging" : "idle";

  String payload;
  serializeJson(doc, payload);

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  int responseCode = http.POST(payload);
  Serial.printf("Port %d telemetry sent, response: %d\n", portNum, responseCode);
  http.end();
}

// ---------- FEEDBACK HELPERS ----------
void beep(int times, int durationMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(durationMs);
    digitalWrite(BUZZER_PIN, LOW);
    delay(durationMs);
  }
}

void showMessage(String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
}
