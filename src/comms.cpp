// Blynk configuration must be defined before the Blynk header is included.
#include "secrets.h"     // BLYNK_TEMPLATE_ID / NAME / AUTH_TOKEN, WiFi AP creds
#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <BlynkSimpleEsp32.h>
#include <esp_sleep.h>

#include "comms.h"
#include "sensor_data.h"
#include "config.h"

static char auth[] = BLYNK_AUTH_TOKEN;

void commsBegin() {
  WiFiManager wm;
  wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT_S);

  // Open a config AP if no credentials are stored. Blocks until connected
  // or the portal times out.
  if (!wm.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD)) {
    Serial.println("Wifi failed");
    delay(3000);
    ESP.restart();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Blynk.config(auth);
    if (Blynk.connect(BLYNK_CONNECT_TIMEOUT_MS)) {
      Serial.println("Blynk connected");
    } else {
      Serial.println("Blynk failed to connect");
    }
  }
}

void blynkTask(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(dataReadySemaphore, portMAX_DELAY) == pdTRUE) {
      // Average the collected batch.
      float tempSum = 0, humidSum = 0;
      int   gasSum = 0;
      for (int i = 0; i < SAMPLE_COUNT; i++) {
        tempSum  += tempData[i];
        humidSum += humidData[i];
        gasSum   += gasData[i];
      }

      float tempMean  = tempSum / (float)SAMPLE_COUNT;
      float humidMean = humidSum / (float)SAMPLE_COUNT;
      int   gasMean   = gasSum / SAMPLE_COUNT;

      Blynk.virtualWrite(VPIN_TEMP, tempMean);
      Blynk.virtualWrite(VPIN_HUMID, humidMean);
      Blynk.virtualWrite(VPIN_GAS, gasMean);

      if (gasMean >= GAS_ALERT_THRESHOLD) {
        Blynk.logEvent(GAS_ALERT_CODE, GAS_ALERT_MESSAGE);
      }

      vTaskDelay(pdMS_TO_TICKS(2000));  // Let the Blynk transmission finish

      // --- Enter true deep sleep ---
      // Cut power to all sensors by switching off the MOSFET, so they draw
      // zero current while the MCU sleeps. Without this the sensors would
      // keep drawing current even though the ESP32 itself is asleep.
      digitalWrite(POWER_PIN, LOW);
      vTaskDelay(pdMS_TO_TICKS(SLEEP_SETTLE_MS));

      esp_sleep_enable_timer_wakeup(DEEP_SLEEP_US);
      esp_deep_sleep_start();  // The board resets when it wakes
    }
  }
}
