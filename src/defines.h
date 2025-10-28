#define SRC_DEFINES_H_
#pragma once

#include <cstdint>
#include <WiFi.h>
#include <WiFiUdp.h>

struct SuenioCFG {
  uint8_t horas_suenio = 8;
  bool    alarma_on    = true;
  bool    luz_on       = true;
  bool    ok           = false;
};

// ===== WiFi + UDP =====
static const char* WIFI_SSID = "Santiago WiFi";
static const char* WIFI_PASS = "santiago0701";
static IPAddress PC_IP(192,168,100,220);       // <-- IP de la PC
static const uint16_t PC_UDP_PORT = 5005;        // <-- puerto de escucha (nc -ul 5005)

static WiFiUDP udp;