#include "defines.h"
#include "pc_wifi/pc_wifi.h"
#include "lpc_uart/lpc_uart.h"
#include "lpc_config/lpc_config.h"


#define UART_RX_PIN 25
#define UART_TX_PIN 26
#define UART_BAUD   9600

#define LPC_Serial Serial2

SuenioCFG suenio_cfg;
WiFiUDP udp;

void setup() {
  Serial.begin(9600); // debug
  while (!Serial) ; 

  Serial.println("=== ESP32 INIT ===");
  // WiFi arranque (no bloqueante si falla) 
  startWiFi(udp);

  // Iniciar UART2 (LPC845)
  LPC_Serial.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
}

void loop() {
  // Leer bytes del UART (LPC845) y buscar comandos entre <>
  Procesar_Comandos(&LPC_Serial, &suenio_cfg, udp);

  // Leer comandos UDP desde la PC
  Procesar_Comandos_UDP(udp, &LPC_Serial, &suenio_cfg);

  // Asegurar conexion WiFi
  wifiEnsureConnected();
}



