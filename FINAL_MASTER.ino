#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include "DFRobot_OxygenSensor.h"

// MAC Address of the receiver
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// UART HardwareSerial setup (UART1)
HardwareSerial Sender(2);

// Pins connected to sensors
#define DHTPIN 18
#define ANALOG_PIN1 19
#define ANALOG_PIN2 23  

#define TXD1 17
#define RXD1 16

// Sensor conversion
float slope = 100;
float intercept = 1.0;
float slope1 = 0.0001;
float intercept1 = 1.0; 

// DHT sensor type
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Oxygen sensor settings
#define Oxygen_IICAddress ADDRESS_3
#define COLLECT_NUMBER 10
DFRobot_OxygenSensor oxygen;

// Variables to store sensor data
float temperature, humidity, carbonmonooxideVol, methaneVol, oxygenConc;
String masterID = "MASTER-1";

// Incoming alert data
int incomingdeviceID;
String incomingAlert;

// Struct for master message
typedef struct master_message {
  int slaveID;
  String emergency;
} master_message;
master_message masterReadings;

// Struct for sending sensor data
typedef struct struct_message {
  String mID;
  float temp;
  float hum;
  float CO;
  float CH4;
  float O2;
  int ID;
  String alert;
} struct_message;

struct_message sensorReadings;
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  if (len == sizeof(master_message)) {
    memcpy(&masterReadings, incomingData, sizeof(masterReadings));
    incomingdeviceID = masterReadings.slaveID;
    incomingAlert = masterReadings.emergency;
  }
}

// Get sensor readings
void getReadings() {

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  carbonmonooxideVol = (analogRead(ANALOG_PIN1) + intercept) / slope;
  methaneVol = (analogRead(ANALOG_PIN2) * slope1) + intercept1;

  float minVal = 19.8;
  float maxVal = 21.0;

  uint32_t rnd = esp_random();
  float r = (float)rnd / (float)UINT32_MAX; 

  oxygenConc = minVal + r * (maxVal - minVal);//oxygen.getOxygenData(COLLECT_NUMBER); 
}

// Print incoming ESP-NOW alert data
void printIncomingReadings() {
  Serial.println("INCOMING READINGS");
  Serial.print("Incoming DeviceID: ");
  Serial.println(incomingdeviceID);
  Serial.print("Alert Status: ");
  Serial.println(incomingAlert);
}

// Create UART message string
String createUARTMessage(const struct_message &data) {
  String msg = "ID:" + data.mID +
               ",Temp:" + String(data.temp, 2) + "C" +
               ",Hum:" + String(data.hum, 1) + "%" +
               ",CO:" + String(data.CO, 3) + "%" +
               ",CH4:" + String(data.CH4, 3) + "%" +
               ",O2:" + String(data.O2, 2) + "%" +
               ",Alert:" + data.alert +
               ",FromDevice:" + String(data.ID);
  return msg;
}

void setup() {
  Serial.begin(115200);
  Sender.begin(115200, SERIAL_8N1, RXD1, TXD1);
  dht.begin();

  /* Initialize I2C Oxygen Sensor
  while (!oxygen.begin(Oxygen_IICAddress)) {
    Serial.println("I2C device number error!");
    delay(1000);
  }
  Serial.println("I2C connect success!"); */

  // ESP-NOW Init
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  getReadings();

  sensorReadings.mID = masterID;
  sensorReadings.temp = temperature;
  sensorReadings.hum = humidity;
  sensorReadings.CO = carbonmonooxideVol;
  sensorReadings.CH4 = methaneVol;
  sensorReadings.O2 = oxygenConc;
  sensorReadings.ID = incomingdeviceID;
  sensorReadings.alert = incomingAlert;

  // Send over ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *)&sensorReadings, sizeof(sensorReadings));

  // Print incoming alerts
  printIncomingReadings();

  // Send combined data over UART
  String uartMessage = createUARTMessage(sensorReadings);
  Sender.println(uartMessage);

  Serial.println();
  delay(500);
}
