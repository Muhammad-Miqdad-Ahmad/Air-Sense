#pragma once

// =============================================================================
//  sensor_data.h  --  Shared sample buffers and synchronization
// =============================================================================
//  These buffers are filled by the sensor task and drained by the Blynk task.
//  The binary semaphore signals "a full batch of SAMPLE_COUNT readings is
//  ready to be averaged and uploaded".
// =============================================================================

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config.h"

extern int   gasData[SAMPLE_COUNT];
extern float tempData[SAMPLE_COUNT];
extern float humidData[SAMPLE_COUNT];

// Given by sensorTask when a batch is ready, taken by blynkTask.
extern SemaphoreHandle_t dataReadySemaphore;

// Creates the semaphore. Call once during setup, before the tasks start.
void sensorDataInit();
