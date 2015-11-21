#include <KnxTpUart.h>

KnxTpUart knx(&Serial, "1.1.199");

void setup() {
  Serial.begin(19200);
  UCSR0C = UCSR0C | B00100000;
  knx.uartReset();
}

void loop() {
   knx.groupReadBool("2/6/0");  // Zum Senden einer Leseanfrage vom Arduino zum KNX auf der Gruppenadresse "2/6/0"
}

void serialEvent() {
  KnxTpUartSerialEventType eType = knx.serialEvent();
}