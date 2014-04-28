#include <KnxTpUart.h>

// Define group address to react on
String my_address = "0/0/100";

// Initialize the KNX TP-UART library on the Serial1 port of Arduino Mega
KnxTpUart knx(&Serial1, "15.15.20");

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
  knx.addListenGroupAddress(my_address);
}


void loop() {
}

void serialEvent1() {
  KnxTpUartSerialEventType eType = knx.serialEvent();
  if (eType == KNX_TELEGRAM) {
     KnxTelegram* telegram = knx.getReceivedTelegram();

     // Is it a read request?
     if (telegram->getCommand() == KNX_COMMAND_READ) {
        //knx.groupAnswerBool(my_address, true);
        knx.groupAnswer2ByteFloat(my_address, 25.28);
     }   
  }
}







