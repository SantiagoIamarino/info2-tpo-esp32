#include "defines.h"


void wifiEnsureConnected();
bool startUDP(uint16_t port);
void startWiFi();
bool Esperar_CFG(SuenioCFG* cfg, uint32_t timeout_ms);
