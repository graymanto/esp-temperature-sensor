#include <Arduino.h>
#include <DallasTemperature.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <stdio.h>
#include <time.h>

// Use pin D4 or GPIO2 for ESP01
#ifdef NODEMCUV2
#define ONE_WIRE_BUS D4
#elif defined ESP01
#define ONE_WIRE_BUS 2
#else
#define ONE_WIRE_BUS D4
#endif

#define SENSOR_METRIC_BASENAME "temperature"
#define METRIC_NAME_SEP "."
#define UNUSED_INDEXES ".0.0"
#define METRIC_NAME                                                            \
  SENSOR_METRIC_BASENAME METRIC_NAME_SEP SENSOR_NUMBER UNUSED_INDEXES

static const unsigned long TIMEOUT = 80000; // ms

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float Celcius = 0;
float Fahrenheit = 0;

void setup() {
  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
  Serial.begin(9600);
  sensors.begin();

  auto startTime = millis();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime >= TIMEOUT) {
      ESP.deepSleep(SLEEP_TIME);
    }

    Serial.print(".");
    delay(1000);
  }

  configTime(0, 0, TIME_SERVER, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    if (millis() - startTime >= TIMEOUT) {
      ESP.deepSleep(SLEEP_TIME);
    }
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
}

void loop() {
  sensors.requestTemperatures();
  Celcius = sensors.getTempCByIndex(0);
  Serial.print(" C  ");
  Serial.print(Celcius);

  time_t now = time(nullptr);
  Serial.println(ctime(&now));

  HTTPClient http;
  http.begin(POST_URL);
  http.addHeader("Content-Type", "text/plain");

  String metricValue =
      String(METRIC_NAME) + " " + String(Celcius) + " " + String(now);
  Serial.println("Posting" + metricValue);
  auto httpCode = http.POST(metricValue);
  http.end();

  auto message = String("Http response was ") + httpCode;
  Serial.println(message);

  Serial.println("Going to sleep");
  ESP.deepSleep(SLEEP_TIME);
}
