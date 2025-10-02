#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Buzzer & Button pins
#define button 4
#define buzzerPin 5

int buttonState = 0;
int deviceID = 1;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Thresholds
const float TEMP_THRESHOLD = 45.0;
const float HUM_THRESHOLD = 90.0;
const float CO_THRESHOLD = 3.0;
const float CH4_THRESHOLD = 12.0;
const float OXYGEN_LOW_THRESHOLD = 19.7;

// Incoming sensor values
float incomingTemp;
float incomingHum;
float incomingCO;
float incomingMethane;
float incomingOxygen;
int incomingdeviceID;
String incomingAlert;
String incomingMasterID;
String success;

// Structs
typedef struct master_message {
  int slaveID;
  String emergency;
} master_message;

master_message masterReadings;

typedef struct struct_message {
  String mID;
  float temp;
  float hum;
  float CO;
  float CH4;
  float oxygen;
  int ID;
  String alert;
} struct_message;

struct_message sensorReadings;
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

const unsigned long timeoutReadingsPeriod = 1000;
const unsigned long timeoutAlertPeriod = 250;

unsigned long lastDataReceivedTime = 0;
unsigned long lastAlertReceivedTime = 0;

bool newDataReceived = false;
bool newAlertReceived = false;

void resetMasterReadings() {
  incomingMasterID = "";
  incomingTemp = 0.0;
  incomingHum = 0.0;
  incomingCO = 0.0;
  incomingMethane = 0.0;
  incomingOxygen = 0.0;
}

void resetAlertReadings() {
  incomingdeviceID = 0;
  incomingAlert = "";
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));

  incomingMasterID = incomingReadings.mID;
  incomingTemp = incomingReadings.temp;
  incomingHum = incomingReadings.hum;
  incomingCO = incomingReadings.CO;
  incomingMethane = incomingReadings.CH4;
  incomingOxygen = incomingReadings.oxygen;
  incomingdeviceID = incomingReadings.ID;
  incomingAlert = incomingReadings.alert;

  newDataReceived = true;
  lastDataReceivedTime = millis();
}

void printIncomingReadings() {
  Serial.println();
  Serial.println("INCOMING READINGS");
  Serial.print("Incoming Master-ID: ");
  Serial.println(incomingMasterID);
  Serial.print("Temperature: ");
  Serial.print(incomingTemp);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(incomingHum);
  Serial.println(" %");
  Serial.print("CO level: ");
  Serial.println(incomingCO);
  Serial.print("Methane Level: ");
  Serial.println(incomingMethane);
  Serial.print("Oxygen Level: ");
  Serial.println(incomingOxygen);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("INCOMING READINGS");

  display.setCursor(0, 10);
  display.print("Temp: ");
  display.print(incomingTemp);
  display.print("C");

  display.setCursor(0, 20);
  display.print("Humidity: ");
  display.print(incomingHum);
  display.print("%");

  display.setCursor(0, 30);
  display.print("CO: ");
  display.print(incomingCO);
  display.print("%");

  display.setCursor(0, 40);
  display.print("CH4: ");
  display.print(incomingMethane);
  display.print("%");

  display.setCursor(0, 50);
  display.print("O2: ");
  display.print(incomingOxygen);
  display.print("%");

  display.display();
}

void checkThresholdAndAlert() {
  bool alert = false;
  String alertMsg = "Threshold: ";

  if (incomingTemp > TEMP_THRESHOLD) {
    alert = true;
    alertMsg += "Temp ";
  }
  if (incomingHum > HUM_THRESHOLD) {
    alert = true;
    alertMsg += "Hum ";
  }
  if (incomingCO > CO_THRESHOLD) {
    alert = true;
    alertMsg += "CO ";
  }
  if (incomingMethane > CH4_THRESHOLD) {
    alert = true;
    alertMsg += "CH4 ";
  }
  if (incomingOxygen < OXYGEN_LOW_THRESHOLD) {
    alert = true;
    alertMsg += "O2 ";
  }

  if (alert) {
    Serial.println(alertMsg);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.println(alertMsg);
    display.display();

    digitalWrite(buzzerPin, HIGH);
    delay(4000);
    digitalWrite(buzzerPin, LOW);
  }
}

void alertSystem() {
  buttonState = digitalRead(button);

  if (buttonState == HIGH) {
    sensorReadings.ID = deviceID;
    sensorReadings.alert = "codeRed";

    esp_now_send(broadcastAddress, (uint8_t *)&sensorReadings, sizeof(sensorReadings));

    masterReadings.slaveID = deviceID;
    masterReadings.emergency = "codeRed";

    esp_now_send(broadcastAddress, (uint8_t *)&masterReadings, sizeof(masterReadings));
  } else {
    masterReadings.slaveID = deviceID;
    masterReadings.emergency = "noProblem";
    esp_now_send(broadcastAddress, (uint8_t *)&masterReadings, sizeof(masterReadings));

    sensorReadings.ID = deviceID;
    sensorReadings.alert = "noProblem";

    esp_now_send(broadcastAddress, (uint8_t *)&sensorReadings, sizeof(sensorReadings));
  }

  Serial.print("Alert Status: ");
  Serial.print(incomingAlert);
  Serial.print(" - From Device: ");
  Serial.println(incomingdeviceID);
}

void resetIfNoData() {
  if (!newDataReceived && millis() - lastDataReceivedTime > timeoutReadingsPeriod) {
    resetMasterReadings();
  }
  newDataReceived = false;
}

void resetAlertData() {
  if (!newAlertReceived && millis() - lastAlertReceivedTime > timeoutAlertPeriod) {
    resetAlertReadings();
  }
  newAlertReceived = false;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(button, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
}

void loop() {
  printIncomingReadings();
  checkThresholdAndAlert();
  resetIfNoData();
  resetAlertData();
  alertSystem();
  delay(500);
}
