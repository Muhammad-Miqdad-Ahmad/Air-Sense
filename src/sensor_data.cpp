#include "sensor_data.h"

// Definitions of the shared buffers declared in sensor_data.h
int   gasData[SAMPLE_COUNT];
float tempData[SAMPLE_COUNT];
float humidData[SAMPLE_COUNT];

SemaphoreHandle_t dataReadySemaphore = nullptr;

void sensorDataInit() {
  // Binary semaphore: producer (sensorTask) gives, consumer (blynkTask) takes.
  dataReadySemaphore = xSemaphoreCreateBinary();
}
