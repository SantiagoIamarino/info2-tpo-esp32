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
  
  char buf[80];
	//parseo perfil a string de 2 dígitos ejemplo 1 = 01, 10 = 10
	char pf[3];
	int p = suenio_cfg->profile_id;
	pf[0] = '0' + (p / 10);
	pf[1] = '0' + (p % 10);
	pf[2] = '\0';

	// parseo horas de sueño a string de 2 dígitos
  char hs[3];
  int h = suenio_cfg->horas_suenio;
  if (h < 0) h = 0; else if (h > 20) h = 20;
  // 2 digitos para la hora
  hs[0] = '0' + (h / 10);
  hs[1] = '0' + (h % 10);
  hs[2] = '\0';

  snprintf(buf, sizeof(buf),
           "<CFG:PF_ID=%s;HORAS_SUENIO=%s;ALARMA_ON=%s;LUZ_ON=%s>",
           pf,
           hs,
           suenio_cfg->alarma_on ? "0TRUE" : "FALSE",
           suenio_cfg->luz_on    ? "0TRUE" : "FALSE");

  lpc_serial->println(buf);
  lpc_serial->println("\r\n");
}

void Enviar_Info_Fisio_PC(WiFiUDP& udp, const char* cmd) {
  // Reenviar tal cual al PC
  udp.beginPacket(PC_IP, PC_UDP_PORT);
  udp.write((const uint8_t*)cmd, strlen(cmd));
  udp.endPacket();
}

void Esperar_ACK_PC(WiFiUDP& udp, char* ack_str, uint32_t timeout_ms) {
  uint32_t t0 = millis();
  char rx[160];
  while (millis() - t0 < timeout_ms) {
    int n = udp.parsePacket();
    if (n > 0) {
      if (n >= (int)sizeof(rx)) n = sizeof(rx) - 1;
      int m = udp.read((uint8_t*)rx, n);
      rx[m] = '\0';
      if (strcmp(rx, ack_str) == 0) {
        Serial.println("ACK recibido");
        return;
      }
    }
    // pequeño delay para no bloquear WiFi
    delay(2);
  }
}

void Enviar_ACK_LPC(HardwareSerial* lpc_serial, const char* ack_str) {
  lpc_serial->println(ack_str);
}

void handleCommand(const char* cmd, HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg, WiFiUDP& udp) {
  Serial.print("Comando recibido: ");
  Serial.println(cmd);

  if(strcmp(cmd, "PING") == 0){
    lpc_serial->println("<PONG>");
  }
  else if (strcmp(cmd, "REQ_CONFIG") == 0) {
    Obtener_Config_PC(suenio_cfg, udp, 2000);
    Enviar_ACK_LPC(lpc_serial, "<ACK_REQ_CONFIG>");
    Enviar_Config_LPC(lpc_serial, suenio_cfg);
  }
  else if (strncmp(cmd, "INFO_FISIO", 10) == 0) {
    String formatted_cmd = "<" + String(cmd) + ">";
    Enviar_Info_Fisio_PC(udp, formatted_cmd.c_str());
    Esperar_ACK_PC(udp, "<ACK_INFO_FISIO>", 2000);
    Enviar_ACK_LPC(lpc_serial, "<ACK_INFO_FISIO>");
  }
  else {
    //lpc_serial->println("<ERR:UNKNOWN_CMD>");
  }
}

void Procesar_Comandos(HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg, WiFiUDP& udp) {
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
      handleCommand(frame, lpc_serial, suenio_cfg, udp);   // procesar comando
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
