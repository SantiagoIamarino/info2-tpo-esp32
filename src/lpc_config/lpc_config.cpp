#include "lpc_config.h"


// --- helpers ---
static bool parseBoolToken(const char* s) {
  // acepta "TRUE", "FALSE", y tu variante "0TRUE"
  if (!s) return false;
  if (strncmp(s, "0TRUE", 5) == 0) return true;
  if (strncmp(s, "TRUE", 4) == 0)  return true;
  return false; // cualquier otra cosa la tomo como false
}

bool parseConfigFrame(const char* frame, SuenioCFG* out) {
  Serial.println("parseConfigFrame");
  // Parsear solo si es un CFG
  if (strncmp(frame, "<CFG:", 5) != 0) {
    return false;
  }

  Serial.println("Espera cfg");

  // Formato: <CFG:HORAS_SUENIO=08;ALARMA_ON=TRUE;LUZ_ON=TRUE>
  if (!frame || strncmp(frame, "<CFG:", 5) != 0) return false;

  // Copio a un buffer editable para usar strtok (opcional)
  char buf[128];
  strncpy(buf, frame, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  // Quito '<' y '>'
  char* start = strchr(buf, ':');        // después de "<CFG:"
  char* end   = strrchr(buf, '>');       // '>'
  if (!start || !end) return false;
  *end = '\0';
  ++start;

  // Busco cada campo por clave
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

void Enviar_REQ_CONFIG() {
  const char* payload = "<REQ_CONFIG>";
  udp.beginPacket(PC_IP, PC_UDP_PORT);
  udp.write((const uint8_t*)payload, strlen(payload));
  udp.endPacket();
}

bool Obtener_Config_PC(SuenioCFG* suenio_cfg, uint32_t timeout_ms) {
  Enviar_REQ_CONFIG();
  return Esperar_CFG(suenio_cfg, timeout_ms);
}
