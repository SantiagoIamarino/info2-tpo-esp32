#include <HardwareSerial.h>


void Enviar_Config_LPC(HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg, bool es_update = false);
void handleCommand(const char* cmd, HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg, WiFiUDP& udp);
void Procesar_Comandos(HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg, WiFiUDP& udp);
void Esperar_ACK_PC(WiFiUDP& udp, char* ack_str, uint32_t timeout_ms);