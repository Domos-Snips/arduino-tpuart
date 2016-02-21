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
  knx.addListenGroupAddress("3/0/1");
  knx.addListenGroupAddress("3/0/2");
  knx.addListenGroupAddress("3/0/3");
  knx.addListenGroupAddress("3/0/4");
  knx.addListenGroupAddress("3/0/5");
  knx.addListenGroupAddress("3/0/6");
  knx.addListenGroupAddress("3/0/7");
  knx.addListenGroupAddress("3/0/8");
}


void loop() {
}

void serialEvent1() {
  KnxTpUartSerialEventType eType = knx.serialEvent();
  if (eType == KNX_TELEGRAM) {
    KnxTelegram* telegram = knx.getReceivedTelegram();

    String target =
      String(0 + telegram->getTargetMainGroup())   + "/" +
      String(0 + telegram->getTargetMiddleGroup()) + "/" +
      String(0 + telegram->getTargetSubGroup());

    // Ist es eine Leseanfrage ?
    if (telegram->getCommand() == KNX_COMMAND_READ) {
    
      // Ist Ziel gleich Gruppenadresse 3/0/1 ?
      if (target == "3/0/1") {
        knx.groupAnswerBool("3/0/1", true);
        // Ausgabe : 1
      }
      // Ist Ziel gleich Gruppenadresse 3/0/2 ?
      if (target == "3/0/2") {
        knx.groupAnswer1ByteInt("3/0/2", 126);
        // Ausgabe : 49%
      }
      // Ist Ziel gleich Gruppenadresse 3/0/3 ?
      if (target == "3/0/3") {
        knx.groupAnswer2ByteInt("3/0/3", 1000);
        // Ausgabe : 10
      }
      // Ist Ziel gleich Gruppenadresse 3/0/4 ?
      if (target == "3/0/4") {
        knx.groupAnswer2ByteFloat("3/0/4", 25.28);
        // Ausgabe : 25,28
      }
      // Ist Ziel gleich Gruppenadresse 3/0/5 ?
      if (target == "3/0/5") {
        knx.groupAnswer3ByteTime("3/0/5", 7, 0, 0, 1);
        // Ausgabe : Sonntag 00:00:01
      }
      // Ist Ziel gleich Gruppenadresse 3/0/6 ?
      if (target == "3/0/6") {
        knx.groupAnswer3ByteDate("3/0/6", 31, 1, 3);
        // Ausgabe : 31.01.2003
      }
      // Ist Ziel gleich Gruppenadresse 3/0/7 ?
      if (target == "3/0/7") {
        knx.groupAnswer4ByteFloat("3/0/7", -100);
        // Ausgabe : -100
      }
      // Ist Ziel gleich Gruppenadresse 3/0/8 ?
      if (target == "3/0/8") {
        knx.groupAnswer14ByteText("3/0/8", "Hallo");
        // Ausgabe : "Hallo"
      }
    }
  }
}
