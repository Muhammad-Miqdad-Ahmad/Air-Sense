#include <Arduino.h>
#include <DHT.h>

#include "sensors.h"
#include "sensor_data.h"
#include "config.h"

static DHT dht(DHTPIN, DHTTYPE);

void sensorsBegin() {
  pinMode(POWER_PIN, OUTPUT);     // Controls the sensor-power MOSFET
  digitalWrite(POWER_PIN, HIGH);  // Power the sensors on
  dht.begin();
}

void sensorTask(void *pvParameters) {
  int dataIndex = 0;

  while (1) {
    digitalWrite(POWER_PIN, HIGH);  // Make sure the sensors are powered

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Development fallback: substitute a random value when the DHT11 is
    // missing or returns NaN so the dashboard still shows live data.
    if (isnan(t)) t = random(1, 56);
    if (isnan(h)) h = random(1, 56);

    // Oversample the analog gas sensor to smooth out noise.
    int sum = 0;
    for (int i = 0; i < GAS_OVERSAMPLE; i++) {
      sum += analogRead(GAS_SENSOR_PIN);
    }
    int gasValue = sum / GAS_OVERSAMPLE;

    tempData[dataIndex]  = t;
    humidData[dataIndex] = h;
    gasData[dataIndex]   = gasValue;
    dataIndex++;

    if (dataIndex >= SAMPLE_COUNT) {
      dataIndex = 0;                        // Reset for the next batch
      xSemaphoreGive(dataReadySemaphore);   // Tell blynkTask a batch is ready
      vTaskDelay(pdMS_TO_TICKS(500));
    }

    vTaskDelay(pdMS_TO_TICKS(SENSOR_INTERVAL_MS));
  }
}
