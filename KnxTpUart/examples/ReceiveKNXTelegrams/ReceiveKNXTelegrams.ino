#include <KnxTpUart.h>

// Initialize the KNX TP-UART library on the Serial1 port of Arduino Mega
KnxTpUart knx(&Serial1);

void setup() {
  Serial.begin(9600);
  Serial.println("TP-UART Test");  

  Serial1.begin(19200);
  UCSR1C = UCSR1C | B00100000; // Even Parity

  Serial.print("UCSR1A: ");
  Serial.println(UCSR1A, BIN);

  Serial.print("UCSR1B: ");
  Serial.println(UCSR1B, BIN);

  Serial.print("UCSR1C: ");
  Serial.println(UCSR1C, BIN);

  knx.uartReset();
}


void loop() {
}

void serialEvent1() {
  KnxTpUartSerialEventType eType = knx.serialEvent();
  if (eType == TPUART_RESET_INDICATION) {
     Serial.println("Event TPUART_RESET_INDICATION"); 
  } else if (eType == TPUART_STATE_INDICATION) {
     Serial.println("Event TPUART_STATE_INDICATION"); 
  } else if (eType == UNKNOWN) {
    Serial.println("Event UNKNOWN");
  } else if (eType == KNX_TELEGRAM) {
     Serial.println("Event KNX_TELEGRAM");
     KNXTelegram* telegram = knx.getReceivedTelegram();
     
     // Here you have the telegram and can do whatever you want
  }
}








