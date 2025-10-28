#include "defines.h"


void wifiEnsureConnected();
bool startUDP(uint16_t port, WiFiUDP& udp);
void startWiFi( WiFiUDP& udp );
bool Esperar_CFG(SuenioCFG* cfg, WiFiUDP& udp, uint32_t timeout_ms);
