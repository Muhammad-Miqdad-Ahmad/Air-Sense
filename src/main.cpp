// =============================================================================
//  Air-Sense  --  ESP32-C3 air quality monitor
// =============================================================================
//  Reads temperature, humidity and gas, averages a batch of samples, uploads
//  the means to the Blynk cloud, then cuts sensor power and deep sleeps.
//
//  Code layout:
//    config.h / secrets.h  - pins, tuning constants, credentials
//    sensor_data.*         - shared sample buffers + semaphore
//    sensors.*             - sensor init and acquisition task
//    comms.*               - WiFi/Blynk setup, upload task, deep sleep
// =============================================================================

#include <Arduino.h>

#include "config.h"
#include "sensor_data.h"
#include "sensors.h"
#include "comms.h"

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give the serial port time to come up

  sensorsBegin();      // Power on sensors, init DHT
  commsBegin();        // WiFi + Blynk
  sensorDataInit();    // Create the data-ready semaphore

  // Producer: samples sensors (core 0).
  xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 1, NULL, 0);

  // Consumer: uploads to Blynk and manages deep sleep (core 0).
  xTaskCreatePinnedToCore(blynkTask, "BlynkTask", 4096, NULL, 5, NULL, 0);
}

void loop() {
  // All work happens in the FreeRTOS tasks.
}
