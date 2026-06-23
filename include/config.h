#pragma once

// =============================================================================
//  config.h  --  Hardware pins and tuning constants
// =============================================================================
//  Non-secret, safe-to-commit configuration for the Air-Sense node.
//  Credentials live in "secrets.h" instead.
// =============================================================================

// --- Sensor pins (ESP32-C3) ---
#define DHTPIN          2     // DHT11 data line
#define DHTTYPE         DHT11 // DHT sensor model
#define GAS_SENSOR_PIN  0     // Analog gas sensor (ADC)

// --- Power-switch pin ---
//  Drives the gate of a MOSFET that supplies power to all the sensors.
//  HIGH  -> MOSFET on  -> sensors powered.
//  LOW   -> MOSFET off -> sensors completely disconnected from power.
//  We pull this LOW right before deep sleep so the sensors draw ZERO current,
//  giving a true low-power deep sleep instead of just idling the MCU.
#define POWER_PIN       10

// --- Sampling / averaging ---
#define SAMPLE_COUNT        5     // Readings averaged before each upload
#define GAS_OVERSAMPLE      10    // ADC reads averaged per gas sample
#define SENSOR_INTERVAL_MS  1000  // Delay between sensor samples

// --- Alerting ---
#define GAS_ALERT_THRESHOLD 800   // Mean gas value that triggers a Blynk event
#define GAS_ALERT_CODE      "gndi_hwa"
#define GAS_ALERT_MESSAGE   "Saan bnd kr lo je sare. hwa saaan len de qabil koi ni je"

// --- Blynk virtual pins ---
#define VPIN_TEMP   V0
#define VPIN_HUMID  V1
#define VPIN_GAS    V2

// --- Connectivity timeouts ---
#define WIFI_PORTAL_TIMEOUT_S   60     // WiFiManager config portal lifetime
#define BLYNK_CONNECT_TIMEOUT_MS 10000 // How long to wait for the Blynk cloud

// --- Deep sleep ---
#define SLEEP_SETTLE_MS  10000                          // Settle time after powering sensors off
#define DEEP_SLEEP_US    (10ULL * 60ULL * 1000000ULL)   // Sleep duration: 10 minutes
