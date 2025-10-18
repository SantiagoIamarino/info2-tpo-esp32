#define UART_RX_PIN 16  
#define UART_TX_PIN 17 
#define UART_BAUD   9600

#define LPC_Serial Serial2

const int horas_suenio = 8;
const bool alarma_on = true;
const bool luz_on = true;

// === Variables para parsing de comandos ===
char frame[64];             // buffer del comando
uint8_t idx = 0;
bool inFrame = false;
uint32_t lastByteMs = 0;
const uint32_t FRAME_TIMEOUT_MS = 300;  // descartar si el frame se corta

void setup() {
  Serial.begin(9600); // debug
  while (!Serial) ; 

  Serial.println("=== ESP32 INIT ===");

  // Iniciar UART2 (LPC845)
  LPC_Serial.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
}

void Enviar_Config_LPC(HardwareSerial* lpc_serial) {
  Serial.println("Accion: enviar configuracion");
  lpc_serial->println("<ACK_REQ_CONFIG>");

  char buf[80];
  char hs[3];
  int h = horas_suenio;
  if (h < 0) h = 0; else if (h > 20) h = 20;
  // 2 digitos para la hora
  hs[0] = '0' + (h / 10);
  hs[1] = '0' + (h % 10);
  hs[2] = '\0';

  snprintf(buf, sizeof(buf),
           "<CFG:HORAS_SUENIO=%s;ALARMA_ON=%s;LUZ_ON=%s>",
           hs,
           alarma_on ? "TRUE" : "FALSE",
           luz_on    ? "TRUE" : "FALSE");

  lpc_serial->println(buf);
  lpc_serial->println("\r\n");
}

void handleCommand(const char* cmd, HardwareSerial* lpc_serial) {
  Serial.print("Comando recibido: ");
  Serial.println(cmd);

  if(strcmp(cmd, "PING") == 0){
    lpc_serial->println("<PONG>");
  }
  else if (strcmp(cmd, "REQ_CONFIG") == 0) {
    Enviar_Config_LPC(lpc_serial);
  }
  else {
    lpc_serial->println("<ERR:UNKNOWN_CMD>");
  }
}

void Procesar_Comandos(HardwareSerial* lpc_serial){
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
      handleCommand(frame, lpc_serial);   // procesar comando
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

void loop() {
  // Leer bytes del UART (LPC845) y buscar comandos entre < 
  Procesar_Comandos(&LPC_Serial);
}



