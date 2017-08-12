/*

   File: ReplyToKNXRead.ino

   Author: Daniel Kleine-Albers (Since 2012)
   Modified: Thorsten Gehrig (Since 2014)
   Modified: Mag Gyver (Since 2016)

   Last modfified: 12.08.2017
   Reason: Added link to library

   Test constellation = ARDUINO MEGA <-> 5WG1 117-2AB12

*/

#include <KnxTpUart.h> // https://bitbucket.org/thorstengehrig/arduino-tpuart-knx-user-forum/overview

// Define physical address
#define KNX_PA "1.1.15"

// Initialize the KNX TP-UART library on the Serial1 port of ARDUINO MEGA
KnxTpUart knx(&Serial1, KNX_PA);

void setup() {
  Serial.begin(9600);
  Serial.println("TP-UART VERSION 12.08.2017");

  Serial1.begin(19200, SERIAL_8E1); // Even parity

  Serial.print("UCSR1A: ");
  Serial.println(UCSR1A, BIN);

  Serial.print("UCSR1B: ");
  Serial.println(UCSR1B, BIN);

  Serial.print("UCSR1C: ");
  Serial.println(UCSR1C, BIN);

  while (!Serial1) {
  }
  knx.uartReset();
  
  // Define group address to react on
  knx.addListenGroupAddress("3/0/0");
  knx.addListenGroupAddress("3/0/1");
  knx.addListenGroupAddress("3/0/2");
  knx.addListenGroupAddress("3/0/3");
  knx.addListenGroupAddress("3/0/4");
  knx.addListenGroupAddress("3/0/5");
  knx.addListenGroupAddress("3/0/6");
  knx.addListenGroupAddress("3/0/7");
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

    // Is it a read request?
    if (telegram->getCommand() == KNX_COMMAND_READ) {

      // Is the destination address equal to group address 3/0/0?
      if (target == "3/0/0") {
        knx.groupAnswerBool("3/0/0", true);
        // Display: 1
      }
      // Is the destination address equal to group address 3/0/1?
      if (target == "3/0/1") {
        knx.groupAnswer1ByteInt("3/0/1", 126);
        // Display: 49%
      }
      // Is the destination address equal to group address 3/0/2?
      if (target == "3/0/2") {
        knx.groupAnswer2ByteInt("3/0/2", 1000);
        // Display: 1000
      }
      // Is the destination address equal to group address 3/0/3?
      if (target == "3/0/3") {
        knx.groupAnswer2ByteFloat("3/0/3", 25.28);
        // Display: 25,28
      }
      // Is the destination address equal to group address 3/0/4?
      if (target == "3/0/4") {
        knx.groupAnswer3ByteTime("3/0/4", 7, 0, 0, 1);
        // Display: Sonntag 00:00:01
      }
      // Is the destination address equal to group address 3/0/5?
      if (target == "3/0/5") {
        knx.groupAnswer3ByteDate("3/0/5", 31, 1, 3);
        // Display: 31.01.2003
      }
      // Is the destination address equal to group address 3/0/6?
      if (target == "3/0/6") {
        knx.groupAnswer4ByteFloat("3/0/6", -100);
        // Display: -100
      }
      // Is the destination address equal to group address 3/0/7?
      if (target == "3/0/7") {
        knx.groupAnswer14ByteText("3/0/7", "Hallo");
        // Display: "Hallo"
      }
    }
  }
}

/* End of ReplyToKNXRead.ino */