#include "pc_wifi.h"
#include "lpc_config/lpc_config.h"

unsigned long lastWifiAttempt = 0;

void startWiFi(WiFiUDP& udp) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    udp.begin(0);

    wl_status_t wifi_st;
    int timeoutMs = 15000;
    unsigned long t0 = millis();
    while ((wifi_st = WiFi.status()) != WL_CONNECTED && (millis() - t0) < timeoutMs) {
        delay(250);
    }

    if (wifi_st != WL_CONNECTED) {
        Serial.printf("[WiFi] conexion fallo", wifi_st);
    } else {
        Serial.printf("[WiFi] Conectado. IP=%s RSSI=%d dBm\n",
                    WiFi.localIP().toString().c_str(), WiFi.RSSI());

        if (!startUDP(PC_UDP_PORT, udp)) {
            Serial.printf("[PC CONNECTION] Conexión FALLÓ", wifi_st);
        } else {
            Serial.printf("[PC CONNECTION] Conectado a %s:%u\n", PC_IP.toString().c_str(), PC_UDP_PORT);
        }
    }
}

bool startUDP(uint16_t port, WiFiUDP& udp) {
  // 3) WiFiUDP.begin devuelve 1 si ok, 0 si falla
  int ok = udp.begin(port);
  if (!ok) {
    Serial.printf("[UDP] begin(%u) FALLÓ\n", port);
    return false;
  }

  // envio <PING> y espero <PONG> de la PC
  udp.beginPacket(PC_IP, PC_UDP_PORT);
  const char* ping_str = "<PING>";
  udp.write((const uint8_t*)ping_str, strlen(ping_str));
  udp.endPacket();

  uint32_t t0 = millis();
  char rx[160];
  while (millis() - t0 < 3000) {
    int n = udp.parsePacket();
    if (n > 0) {
      if (n >= (int)sizeof(rx)) n = sizeof(rx) - 1;
      int m = udp.read((uint8_t*)rx, n);
      rx[m] = '\0';
      if (strcmp(rx, "<PONG>") == 0) {
        Serial.println("PC (IP: " + udp.remoteIP().toString() + ") conectada correctamente");
        return true;
      }
    }
    // pequeño delay para no bloquear WiFi
    delay(2);
  }

  return false;
}

void wifiEnsureConnected() {
  if (WiFi.status() == WL_CONNECTED) return;
  unsigned long now = millis();
  if (now - lastWifiAttempt < 2000) return; // intenta cada 2 s
  lastWifiAttempt = now;
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
}

// --- Esperar respuesta con timeout (ms) ---
bool Esperar_CFG(SuenioCFG* cfg, WiFiUDP& udp, uint32_t timeout_ms = 2000) {
  uint32_t t0 = millis();
  char rx[160];
  while (millis() - t0 < timeout_ms) {
    int n = udp.parsePacket();
    if (n > 0) {
      if (n >= (int)sizeof(rx)) n = sizeof(rx) - 1;
      int m = udp.read((uint8_t*)rx, n);
      rx[m] = '\0';

      // validar que viene del host esperado
      if (udp.remoteIP() != PC_IP || udp.remotePort() != PC_UDP_PORT) {
        // llega de otro origen: ignorar
        continue;
      }

      if(parseConfigFrame(rx, cfg)) return true;
      
    }
    // pequeño delay para no bloquear WiFi
    delay(2);
  }
  return false; // timeout
}