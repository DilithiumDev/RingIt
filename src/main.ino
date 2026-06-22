#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "certs.h"

#ifndef STASSID
#define STASSID "<your ssid here>"
#define STAPSK "<your password here>"
#endif

#ifndef API_KEY
#define API_KEY "<your api key here>"
#endif

#ifndef DEVICE_ID
#define DEVICE_ID "<your device id here>"
#endif

static const char API_URL[] = "https://joinjoaomgcd.appspot.com/_ah/api/messaging/v1/sendPush?deviceId=" DEVICE_ID "&apikey=" API_KEY "&find=true";
static const uint32_t RTC_MAGIC = 0xDEADBEEF;
static const unsigned long WIFI_TIMEOUT_MS = 15000;

ESP8266WiFiMulti WiFiMulti;

struct {
  uint32_t magic;
} rtcData;

bool waitForWiFi(unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (WiFiMulti.run() == WL_CONNECTED) {
      Serial.println("WiFi connected.");
      return true;
    }
    delay(250);
  }
  return false;
}

void RingIt() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi is not connected. Skipping HTTPS request.");
    return;
  }

  BearSSL::WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;

  Serial.println("[HTTPS] begin...");
  if (https.begin(client, API_URL)) {
    Serial.println("[HTTPS] GET...");
    int httpCode = https.GET();

    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.println("[HTTPS] Unable to connect to API URL.");
  }
}

void setup() {
  Serial.begin(9600);
  delay(100);

  Serial.println();
  Serial.println();
  Serial.println();

  ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData));
  bool coldBoot = (rtcData.magic != RTC_MAGIC);

  if (coldBoot) {
    Serial.println("Cold power up");
    rtcData.magic = RTC_MAGIC;
    ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData));
  } else {
    Serial.println("Woke from deep sleep");
  }

  if (!coldBoot) {
    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(STASSID, STAPSK);
    Serial.printf("Connecting to SSID '%s'...\n", STASSID);

    // Trigger the RingIt function to send the push notification
    if (waitForWiFi(WIFI_TIMEOUT_MS)) {
      RingIt();
    } else {
      Serial.println("WiFi connection timed out.");
    }
  }

  // Enter deep sleep to conserve power
  Serial.println("Entering deep sleep.");
  delay(200);
  ESP.deepSleep(0);
}

void loop() {
    // Loop is required, but not used. yield() allows the ESP8266 to handle background tasks while in deep sleep mode, ensuring a clean shutdown.
    yield();

}
