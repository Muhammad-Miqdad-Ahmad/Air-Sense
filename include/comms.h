#pragma once

// =============================================================================
//  comms.h  --  WiFi provisioning, Blynk cloud, upload + deep sleep
// =============================================================================

// Brings up WiFi (via WiFiManager captive portal) and connects to Blynk.
// Call once in setup().
void commsBegin();

// FreeRTOS task: waits for a ready batch, averages it, uploads to Blynk,
// raises a gas alert if needed, then cuts sensor power and deep sleeps.
void blynkTask(void *pvParameters);
