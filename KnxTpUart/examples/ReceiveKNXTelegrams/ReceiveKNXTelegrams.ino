#include <KnxTpUart.h>

// Initialize the KNX TP-UART library on the Serial1 port of Arduino Mega
// and with KNX physical address 15.15.20
KnxTpUart knx(&Serial1, "15.15.20");

int LED = 13;

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

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
  
  knx.addListenGroupAddress("15/0/0");
  knx.addListenGroupAddress("15/0/1");
  knx.addListenGroupAddress("15/0/2");
  knx.addListenGroupAddress("15/0/3");
  knx.addListenGroupAddress("15/0/4");
  knx.addListenGroupAddress("15/0/5");
}


void loop() {
  // nothing in the loop. This example is only to receive telegrams
}

void serialEvent1() {
  KnxTpUartSerialEventType eType = knx.serialEvent();
  if (eType == TPUART_RESET_INDICATION) {
    Serial.println("Event TPUART_RESET_INDICATION");
  } else if (eType == UNKNOWN) {
    Serial.println("Event UNKNOWN");
  } else if (eType == KNX_TELEGRAM) {
    Serial.println("Event KNX_TELEGRAM");
    KnxTelegram* telegram = knx.getReceivedTelegram();
    // Telegrammauswertung auf KNX (bei Empfang immer notwendig)
    String target =
      String(0 + telegram->getTargetMainGroup())   + "/" +
      String(0 + telegram->getTargetMiddleGroup()) + "/" +
      String(0 + telegram->getTargetSubGroup());

    // Here you have the telegram and can do whatever you want
    if (telegram->getCommand() == KNX_COMMAND_WRITE) {
      // Auswertung des empfangenen KNX-Telegrammes mit Schreibbefehl (Flag) -> Aktion
      if (target == "15/0/0") {
        int received_15_0_0 = telegram->getBool();
		Serial.print("Empfangener wert");
		Serial.println(received_15_0_0);
        if (received_15_0_0) {
          digitalWrite(LED, HIGH);
        }
        else {
          digitalWrite(LED, LOW);
        }
      }
	  if (target == "15/0/1") {
        int received_15_0_1 = telegram->get1ByteIntValue();
		Serial.print("Empfangener Wert:");
		Serial.println(received_15_0_1);
      }
	  if (target == "15/0/2") {
        int received_15_0_2 = telegram->get2ByteIntValue();
		Serial.print("Empfangener Wert:");
		Serial.println(received_15_0_2);
      }
	  if (target == "15/0/3") {
        float received_15_0_3 = telegram->get2ByteFloatValue();
		Serial.print("Empfangener Wert:");
		Serial.println(received_15_0_3);
      }
	  if (target == "15/0/4") {
        float received_15_0_4 = telegram->get4ByteFloatValue();
		Serial.print("Empfangener Wert:");
		Serial.println(received_15_0_4);
      }
	  if (target == "15/0/5") {
        String received_15_0_5 = telegram->get14ByteValue();
		Serial.print("Empfangener Wert:");
		Serial.println(received_15_0_5);
      }
    }
  }
}