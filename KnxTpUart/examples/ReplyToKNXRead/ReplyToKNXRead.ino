#include <KnxTpUart.h>

// Initialize the KNX TP-UART library on the Serial1 port of Arduino Mega
KnxTpUart knx(&Serial1, "1.1.15");

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
  // Define group address to react on
  knx.addListenGroupAddress("3/0/100);
}


void loop() {
}

void serialEvent1() {
  KnxTpUartSerialEventType eType = knx.serialEvent();
  if (eType == KNX_TELEGRAM) {
    KnxTelegram* telegram = knx.getReceivedTelegram();

    // Telegrammauswertung auf KNX (bei Empfang immer notwendig)
    String target =
      String(0 + telegram->getTargetMainGroup())   + "/" +
      String(0 + telegram->getTargetMiddleGroup()) + "/" +
      String(0 + telegram->getTargetSubGroup());

    // Is it a read request?
    if (telegram->getCommand() == KNX_COMMAND_READ) {
      //knx.groupAnswerBool("3/0/100", true);
      knx.groupAnswer2ByteFloat("3/0/100", 25.28);
    }
  }
}
