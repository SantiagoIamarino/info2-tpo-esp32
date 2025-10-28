#include "pc_wifi/pc_wifi.h"
#include <cstring>
#include <cstdlib>


bool Obtener_Config_PC(SuenioCFG* suenio_cfg, WiFiUDP& udp, uint32_t timeout_ms);
bool parseConfigFrame(const char* frame, SuenioCFG* out);
void Enviar_REQ_CONFIG(WiFiUDP& udp);