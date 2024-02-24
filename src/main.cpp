/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp8266-nodemcu-websocket-server-sensor/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/

#include <Arduino.h>

// Web Server libs
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>

// BME680 libs
#include <Adafruit_BME680.h> // to interface with the BME680 sensor
#include <Adafruit_Sensor.h> // to interface with the BME680 sensor
#include <Wire.h>            // to use I2C

// OLED libs
#include <GyverOLED.h>

// MQ135 libs
#include <TroykaMQ.h>

const char *ssid = "TP-Link_0BFD"; // Insert you home WI-FI ssid here
const char *password = "64583129"; // Insert you home WI-FI password here

AsyncWebServer server(80); // Creating Server with port 80
AsyncWebSocket ws("/ws");  // Creating Web socket with default address
JSONVar readings;

unsigned long lastTime = 0;
unsigned long timerDelay = 2000; // Delay between data updating

Adafruit_BME680 bme;
#define SEALEVELPRESSURE_HPA (1013) // Sea level preasure for your current location

// I used the oled display model SSD1306 with resolution 128x64.
// Also available SSH1106 128x64 and SSD1306 128x32 (font resizing is required).
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
int current_screen = 0;
int screens_count = 4;      // count of info screens we want to show on the display
int screens_count_full = 7; // total count of info screens we want to show on the web server page

// * DO NOT USE D3 and D4 FOR BUTTONS !!!
int PIN_CLICK = D7; // Button pin
int PIN_LED = D8;   // Led pin. You can also use one of onboard LEDs, that have pins D0 or D4.

uint8_t btn_prev;

#define PIN_MQ135 A0 // MQ135 analogue pin.
MQ135 mq135(PIN_MQ135);

// Sensors data structure
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

// Just for aesthetic purpose :)
void showLoadState(String info)
{
  oled.home();
  oled.setScale(2);
  oled.print("MeteX v1.0");
  oled.setCursor(0, 3);
  oled.setScale(1);
  oled.print(info + "                ");
  oled.setCursor(0, 5);
  oled.print("Loading...");
}

// Initialize oled display
void initOled()
{
  oled.init();
  oled.clear();
}

// Initialize BME680 sensor
void initBME()
{
  while (!Serial)
    ;
  Serial.println(F("BME680 test"));

  if (!bme.begin())
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1)
      ;
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);

  // Set internal IIR filter to reduce short-term changes in sensor output values
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  // Set gas heater temperature 320 degrees C for 150ms
  bme.setGasHeater(320, 150);
}

// Initialize MQ135 sensor
void initMQ()
{
  mq135.calibrate();
}

// Initialize LittleFS storage
void initFS()
{
  if (!LittleFS.begin())
  {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else
  {
    Serial.println("LittleFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

// Send sensors data to client
void notifyClients(String sensorReadings)
{
  ws.textAll(sensorReadings);
}

// Get sensors data and format it to json string
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

// Initialize Web Socket
void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// Main
void setup()
{
  Serial.begin(9600);

  initOled();
  showLoadState("Initialization");
  delay(500);

  showLoadState("BME680");
  initBME();
  delay(500);

  showLoadState("MQ135");
  initMQ();
  delay(500);

  showLoadState("Wi-Fi");
  initWiFi();
  delay(500);

  showLoadState("LittleFS");
  initFS();
  delay(500);

  showLoadState("Web Socket");
  initWebSocket();
  delay(500);

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

// Show specific data on the display
void displayData(String displayName, String displayData)
{
  oled.home();
  oled.setScale(1);
  // I use extra spaces instead of the oled.clear() to
  // clear the screen, as it causes the screen to blink

  // There is an option to add oled buffer, but it'll just make impossible to start the server
  // TODO - try another oled lib
  oled.print(displayName + "         ");
  oled.setCursor(0, 2);
  oled.setScale(2);
  oled.print(displayData + "         ");
}

// Update data and save to sData structure
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

// Update data and save to sData structure
void updateMQ135Data()
{
  sData.resistance = mq135.readRatio(); // Rs/Ro
  sData.co2 = mq135.readCO2();
}

// Update sensors data and turn on LED during updating
void updateSensors()
{
  digitalWrite(PIN_LED, HIGH);

  updateBME680Data();
  updateMQ135Data();

  digitalWrite(PIN_LED, LOW);
}

// Get sensor data to display
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

// Loop
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