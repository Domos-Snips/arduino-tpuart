#include <KnxTpUart.h>

// Define group address to react on
int my_main_group = 0;
int my_middle_group = 0;
int my_sub_group = 100;

// Initialize the KNX TP-UART library on the Serial1 port of Arduino Mega
KnxTpUart knx(&Serial1, 15, 15, 20);

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
  if (eType == KNX_TELEGRAM) {
     Serial.println("Event KNX_TELEGRAM");
     KnxTelegram* telegram = knx.getReceivedTelegram();

     // Is the telegram for us?
     if (telegram->getTargetMainGroup() == my_main_group
       && telegram->getTargetMiddleGroup() == my_middle_group
       && telegram->getTargetSubGroup() == my_sub_group) {
        
       // Acknowledge
       //knx.sendAck();
         
       // Is it a read request?
       if (telegram->getCommand() == KNX_COMMAND_READ) {
          knx.groupAnswerBool(my_main_group, my_middle_group, my_sub_group, true);
       }   
     }
  }
}







