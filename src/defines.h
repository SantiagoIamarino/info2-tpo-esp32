#define SRC_DEFINES_H_
#pragma once

#include <cstdint>
#include <WiFi.h>
#include <WiFiUdp.h>

struct SuenioCFG {
  uint8_t profile_id   = 1;
  uint8_t horas_suenio = 8;
  bool    alarma_on    = true;
  bool    luz_on       = true;
  bool    ok           = false;
};

// ===== WiFi + UDP =====
static const char* WIFI_SSID = "Santiago WiFi";
static const char* WIFI_PASS = "santiago0701";
//static const char* WIFI_SSID = "moto g75 5G_6207";
///static const char* WIFI_PASS = "charly1234";
static IPAddress PC_IP(192,168,100,245);  // <-- IP de la PC --- ipconfig getifaddr en0
static const uint16_t PC_UDP_PORT = 5005;        // <-- puerto de escucha (nc -ul 5005)