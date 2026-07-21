#define TINY_GSM_MODEM_SIM800
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <TinyGsmClient.h>
#include <Wire.h>
#include "esp_task_wdt.h"

#define APN "cmnet"
#define PORT 80
#define SERVER "api.thingspeak.com"
#define API_KEY "5BHYAPHQ5BGVLHD8"
#define SerialAT Serial1
#define SerialMon Serial
#define RX 16
#define TX 17
#define TINY_GSM_RX_BUFFER 1024

// Sensor + Calibration
Adafruit_ADS1115 ads;
int sensorRating = 15;
double cali_m = 0.244964;
double cali_c = -0.004;

// Timings
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 1 * 60 * 1000UL;  // 1 minute
unsigned long startMillis = 0;
const unsigned long maxUptime = 5 * 60 * 1000UL;     // 5 minutes
int fieldnum = 1;

//GSM
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void send_data(double Reading, int field);
float readSensor();

void setup() {
  SerialMon.begin(115200);
  delay(3000);

  if (!ads.begin()) {
    SerialMon.println("Failed to initialize ADS1115.");
    while (1);
  }

  SerialAT.begin(9600, SERIAL_8N1, RX, TX);
  SerialMon.println("Initializing GSM...");
  modem.restart();
  modem.waitForNetwork(60000L);
  modem.gprsConnect(APN); 
  SerialMon.println("GSM Ready.");

  startMillis = millis();  // Start the runtime counter
  // Register the current task (loop task) to watchdog
  esp_task_wdt_add(NULL);  // NULL means current task
}

void loop() {
  unsigned long currentMillis = millis();

  // Reset after 5 hours
  if (currentMillis - startMillis >= maxUptime) {
    SerialMon.println("5 mins passed. Restarting ESP...");
    delay(1000);
    ESP.restart();
  }

  // Only act when it's time (schedule-based)
  if (currentMillis >= lastSendTime) {
    SerialMon.println("Reading current...");
    double current = readSensor();
    SerialMon.print("Current: ");
    SerialMon.print(current);
    SerialMon.println(" A");

    // Ensure GSM is connected
    if (!modem.isGprsConnected()) {
      SerialMon.println("GPRS not connected. Reconnecting...");
      modem.restart();
      modem.waitForNetwork(60000L);
      modem.gprsConnect(APN);
    }

    send_data(current, fieldnum);

    // Schedule the next reading
    lastSendTime += sendInterval;
  }

  esp_task_wdt_reset();
  delay(1000);
}


float readSensor() {
  int64_t out = 0;
  int N = 1000;
  float time = 0;
  int senseTime = millis();

  for (int i = 0; i < N; i++) {
    int16_t val = ads.readADC_Differential_0_1();
    out += val * val;
    time += (millis() - senseTime);
    senseTime = millis();

    if (i % 100 == 0) {
      esp_task_wdt_reset();  // Feed the watchdog during long loop
    }
  }

  time /= N;
  out /= N;

  float current = sensorRating * (float)sqrt(out) / 1000;
  current = cali_m * current + cali_c;
  return current;
}

void send_data(double Reading, int field) {
  int csq = modem.getSignalQuality();
  SerialMon.print("Signal Quality: ");
  SerialMon.println(csq);

  SerialMon.print("Connecting to ");
  SerialMon.print(SERVER);
  if (!client.connect(SERVER, PORT)) {
    SerialMon.println(" - Connection failed");
    return;
  }

  SerialMon.println(" - Connected");
  String httpRequestData = String("GET /update?api_key=") + API_KEY + "&field" + field + "=" + Reading + " HTTP/1.1\r\nHost: " + SERVER + "\r\nConnection: close\r\n\r\n";
  client.print(httpRequestData);

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    while (client.available()) {
      String line = client.readStringUntil('\n');
      SerialMon.println(line);
      timeout = millis();  // reset timeout
      esp_task_wdt_reset();  // ⚠️ Add this to avoid WDT trigger during long waits
    }
    esp_task_wdt_reset();  // ⚠️ Also reset between `client.connected` loop
  }

  SerialMon.println("Closing connection.");
  client.stop();
}