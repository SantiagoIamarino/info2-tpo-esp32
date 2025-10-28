#include "defines.h"
#include "lpc_uart.h"
#include "pc_wifi/pc_wifi.h"
#include "lpc_config/lpc_config.h"

// === Variables para parsing de comandos ===
char frame[64];             // buffer del comando
uint8_t idx = 0;
bool inFrame = false;
uint32_t lastByteMs = 0;
const uint32_t FRAME_TIMEOUT_MS = 300;  // descartar si el frame se corta


void Enviar_Config_LPC(HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg) {
  Serial.println("Accion: enviar configuracion");
  lpc_serial->println("<ACK_REQ_CONFIG>");

  char buf[80];
  char hs[3];
  int h = suenio_cfg->horas_suenio;
  if (h < 0) h = 0; else if (h > 20) h = 20;
  // 2 digitos para la hora
  hs[0] = '0' + (h / 10);
  hs[1] = '0' + (h % 10);
  hs[2] = '\0';

  snprintf(buf, sizeof(buf),
           "<CFG:HORAS_SUENIO=%s;ALARMA_ON=%s;LUZ_ON=%s>",
           hs,
           suenio_cfg->alarma_on ? "0TRUE" : "FALSE",
           suenio_cfg->luz_on    ? "0TRUE" : "FALSE");

  lpc_serial->println(buf);
  lpc_serial->println("\r\n");
}

void handleCommand(const char* cmd, HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg) {
  Serial.print("Comando recibido: ");
  Serial.println(cmd);

  if(strcmp(cmd, "PING") == 0){
    lpc_serial->println("<PONG>");
  }
  else if (strcmp(cmd, "REQ_CONFIG") == 0) {
    Serial.println("Accion: solicitar configuracion al PC");
    Obtener_Config_PC(suenio_cfg);
    Serial.print("  Horas suenio: "); Serial.println(suenio_cfg->horas_suenio);
    Enviar_Config_LPC(lpc_serial, suenio_cfg);
  }
  else if (strncmp(cmd, "INFO_FISIO", 10) == 0) {
    Serial.print("RECIBO INFO FISIOLOGICA");
  }
  else {
    //lpc_serial->println("<ERR:UNKNOWN_CMD>");
  }
}

void Procesar_Comandos(HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg) {
  while (lpc_serial->available()) {
    char c = (char)lpc_serial->read();
    Serial.println(c);
    lastByteMs = millis();

    if (c == '<') {
      inFrame = true;
      idx = 0;
      continue;
    }
    if (!inFrame) continue;

    if (c == '>') {
      frame[idx] = '\0';      // fin del string
      inFrame = false;
      handleCommand(frame, lpc_serial, suenio_cfg);   // procesar comando
      idx = 0;
      continue;
    }

    // Guardar caracter dentro del frame
    if (idx < sizeof(frame) - 1) {
      frame[idx++] = c;
    } else {
      // overflow: reiniciar
      inFrame = false;
      idx = 0;
    }
  }

  // timeout si no llega el cierre '>'
  if (inFrame && (millis() - lastByteMs > FRAME_TIMEOUT_MS)) {
    inFrame = false;
    idx = 0;
  }
}
