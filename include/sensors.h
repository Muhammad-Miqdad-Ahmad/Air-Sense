#pragma once

// =============================================================================
//  sensors.h  --  Sensor initialization and acquisition task
// =============================================================================

// Powers the sensors and initializes the DHT driver. Call once in setup().
void sensorsBegin();

// FreeRTOS task: samples temperature, humidity and gas, fills the shared
// buffers, and signals the Blynk task once a full batch is collected.
void sensorTask(void *pvParameters);
