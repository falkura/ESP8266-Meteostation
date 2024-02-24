/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp8266-nodemcu-websocket-server-sensor/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <Adafruit_BME680.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SPI.h>
#include <GyverOLED.h>
#include <TroykaMQ.h>

const char *ssid = "TP-Link_0BFD";
const char *password = "64583129";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
JSONVar readings;

unsigned long lastTime = 0;
unsigned long timerDelay = 2000; // 1200

Adafruit_BME680 bme;
#define SEALEVELPRESSURE_HPA (999)

GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
int current_screen = 0;
int screens_count = 4;
int screens_count_full = 7;

int PIN_CLICK = D7; // * DO NOT USE D3 and D4 FOR BUTTONS !!!
int PIN_LED = D8;

uint8_t btn_prev;

#define PIN_MQ135 A0
MQ135 mq135(PIN_MQ135);

struct SensorsData
{
  float temperature;
  float pressure;
  float humidity;
  float gas;
  float altitude;
  int co2;
  float resistance;
} sData;

String dName, dData;

void showLoadState(String info)
{
  // oled.clear();
  oled.home();
  oled.setScale(2);
  oled.print("MeteX v1.0");
  oled.setCursor(0, 3);
  oled.setScale(1);
  oled.print(info + "                ");
  oled.setCursor(0, 5);
  oled.print("Loading...");
}

void initOled()
{
  oled.init();
  oled.clear();
  showLoadState("Initialization");
  delay(500);
}

void initBME()
{
  showLoadState("BME680");

  while (!Serial)
    ;
  Serial.println(F("BME680 test"));

  if (!bme.begin())
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1)
      ;
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  delay(500);
}

void initMQ()
{
  showLoadState("MQ135");
  mq135.calibrate();
  Serial.print("Ro = ");
  Serial.println(mq135.getRo());

  delay(500);
}

String getSensorReadings()
{
  readings["temperature"] = sData.temperature;
  readings["humidity"] = sData.humidity;
  readings["pressure"] = sData.pressure;
  readings["gas"] = sData.gas;
  readings["co2"] = sData.co2;
  readings["altitude"] = sData.altitude;
  readings["resistance"] = sData.resistance;

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Initialize LittleFS
void initFS()
{
  showLoadState("LittleFS");

  if (!LittleFS.begin())
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else
  {
    Serial.println("LittleFS mounted successfully");
  }

  delay(500);
}

// Initialize WiFi
void initWiFi()
{
  showLoadState("Wi-Fi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());

  delay(500);
}

void notifyClients(String sensorReadings)
{
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    String sensorReadings = getSensorReadings();
    Serial.print(sensorReadings);
    notifyClients(sensorReadings);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  showLoadState("Web Socket");

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  delay(500);
}

void setup()
{
  Serial.begin(9600);

  initOled();
  initBME();
  initMQ();
  initWiFi();
  initFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();

  showLoadState("Server");
  delay(500);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_CLICK, INPUT_PULLUP);
  btn_prev = digitalRead(PIN_CLICK);

  oled.clear();
}

void displayData(String displayName, String displayData)
{
  oled.home();
  oled.setScale(1);
  oled.print(displayName + "         ");
  oled.setCursor(0, 2);
  oled.setScale(2);
  oled.print(displayData + "         ");
}

void updateBME680Data()
{
  if (!bme.performReading())
  {
    Serial.println("Failed to perform reading :(");
    return;
  }

  sData.temperature = bme.temperature;
  sData.pressure = bme.pressure / 100.0;
  sData.humidity = bme.humidity;
  sData.gas = bme.gas_resistance / 1000.0;
  sData.altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void updateMQ135Data()
{
  sData.resistance = mq135.readRatio(); // Rs/Ro
  sData.co2 = mq135.readCO2();
}

void updateSensors()
{
  // long read_start_time = millis();
  digitalWrite(PIN_LED, HIGH);

  updateBME680Data();
  updateMQ135Data();

  digitalWrite(PIN_LED, LOW);
  // Serial.print("Total update time: ");
  // Serial.println(millis() - read_start_time);
  // Serial.println();
}

void getSensorData(int _screen, String *dName, String *dData)
{
  switch (_screen)
  {
  case 0:
    *dName = String("Temperature:");
    *dData = String(sData.temperature) + " C";
    break;

  case 1:
    *dName = String("Humidity:");
    *dData = String(sData.humidity) + " %";
    break;

  case 2:
    *dName = String("Pressure:");
    *dData = String(sData.pressure) + " hPa";
    break;

  case 3:
    *dName = String("Gas:");
    *dData = String(sData.gas) + " KOhms";
    break;

  case 4:
    *dName = String("CO2:");
    *dData = String(sData.co2) + " ppm";
    break;

  case 5:
    *dName = String("Ratio:");
    *dData = String(sData.resistance) + " Rs/Ro";
    break;

  case 6:
    *dName = String("Altitude:");
    *dData = String(sData.altitude) + " m";
    break;

  default:
    *dName = String("No Data for S:");
    *dData = String(_screen);
    break;
  }
}

void loop()
{
  int click = digitalRead(PIN_CLICK);

  if (click == LOW && btn_prev == HIGH)
  {
    current_screen++;
    if (current_screen > screens_count)
    {
      current_screen = 0;
    }
  }

  btn_prev = digitalRead(PIN_CLICK);
  getSensorData(current_screen, &dName, &dData);
  displayData(dName, dData);

  if (!bme.performReading())
  {
    Serial.println("Failed to perform reading :(");
    return;
  }

  if ((millis() - lastTime) > timerDelay)
  {
    updateSensors();
    String sensorReadings = getSensorReadings();
    Serial.println(sensorReadings);
    notifyClients(sensorReadings);

    lastTime = millis();
    ws.cleanupClients();
  }

  delay(1);
}