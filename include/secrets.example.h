#pragma once

// =============================================================================
//  secrets.example.h  --  TEMPLATE for credentials
// =============================================================================
//  Copy this file to "secrets.h" (same folder) and replace the placeholder
//  values with your own. "secrets.h" is gitignored so your real tokens stay
//  out of version control.
//
//      cp include/secrets.example.h include/secrets.h
//
//  The Blynk library requires these exact macro names.
// =============================================================================

// --- Blynk cloud credentials ---
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

// --- WiFiManager fallback Access Point (shown when no WiFi is configured) ---
#define WIFI_AP_NAME        "ESP Cyber"
#define WIFI_AP_PASSWORD    "your_ap_password"
