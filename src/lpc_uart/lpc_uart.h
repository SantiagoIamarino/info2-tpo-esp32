#include <HardwareSerial.h>


void Enviar_Config_LPC(HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg);
void handleCommand(const char* cmd, HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg);
void Procesar_Comandos(HardwareSerial* lpc_serial, SuenioCFG* suenio_cfg);