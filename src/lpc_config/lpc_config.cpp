#include "lpc_config.h"
#include "../lpc_uart/lpc_uart.h"

// --- helpers ---
static bool parseBoolToken(const char* s) {
  // acepta "TRUE", "FALSE", y tu variante "0TRUE"
  if (!s) return false;
  if (strncmp(s, "0TRUE", 5) == 0) return true;
  if (strncmp(s, "TRUE", 4) == 0)  return true;
  return false; // cualquier otra cosa la tomo como false
}

bool parseConfigFrame(const char* frame, SuenioCFG* out, bool es_update) {
  const char* principioTrama = (!es_update) ? "<CFG:" : "<CFG_UPDATE:";

  // Parsear solo si es un CFG
  if (!frame || strncmp(frame, principioTrama, strlen(principioTrama)) != 0) {
    return false;
  }

  // Formato: <CFG:PF_ID=01;HORAS_SUENIO=08;ALARMA_ON=TRUE;LUZ_ON=TRUE> o <CFG_UPDATE:...>

  char buf[128];
  strncpy(buf, frame, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  // Quito '<' y '>'
  char* start = strchr(buf, ':');        // después de ":"
  char* end   = strrchr(buf, '>');       // '>'
  if (!start || !end) return false;
  *end = '\0';
  ++start;

  // Busco cada campo por clave
  // PF_ID=NN
  char* p = strstr(start, "PF_ID=");
  if (!p) return false;
  p += strlen("PF_ID=");
  out->profile_id = (uint8_t) strtoul(p, nullptr, 10);

  // HORAS_SUENIO=NN
  char* h = strstr(start, "HORAS_SUENIO=");
  if (!h) return false;
  h += strlen("HORAS_SUENIO=");
  out->horas_suenio = (uint8_t) strtoul(h, nullptr, 10);

  // ALARMA_ON=...
  char* a = strstr(start, "ALARMA_ON=");
  if (!a) return false;
  a += strlen("ALARMA_ON=");
  out->alarma_on = parseBoolToken(a);

  // LUZ_ON=...
  char* l = strstr(start, "LUZ_ON=");
  if (!l) return false;
  l += strlen("LUZ_ON=");
  out->luz_on = parseBoolToken(l);

  out->ok = true;
  return true;
}

void Enviar_REQ_CONFIG(WiFiUDP& udp) {
  const char* payload = "<REQ_CONFIG>";
  udp.beginPacket(PC_IP, PC_UDP_PORT);
  udp.write((const uint8_t*)payload, strlen(payload));
  udp.endPacket();
}

bool Obtener_Config_PC(SuenioCFG* suenio_cfg, WiFiUDP& udp, uint32_t timeout_ms = 2000) {
  Enviar_REQ_CONFIG(udp);
  return Esperar_CFG(suenio_cfg, udp, timeout_ms);
}

void Procesar_Comandos_UDP(WiFiUDP& udp, HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg) {
  int n = udp.parsePacket();
  if (n > 0) {
    char rx[160];
    if (n >= (int)sizeof(rx)) n = sizeof(rx) - 1;
    int m = udp.read((uint8_t*)rx, n);
    rx[m] = '\0';

    Serial.print("Comando UDP recibido: ");
    Serial.println(rx);

    handleCommand(rx, lpc_serial, suenio_cfg, udp);
  }
}
