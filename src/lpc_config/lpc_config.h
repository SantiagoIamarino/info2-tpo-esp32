#include "pc_wifi/pc_wifi.h"
#include <cstring>
#include <cstdlib>


bool Obtener_Config_PC(SuenioCFG* suenio_cfg, uint32_t timeout_ms = 2000);
bool parseConfigFrame(const char* frame, SuenioCFG* out);