#include <KnxTpUart.h>

// Initialize the KNX TP-UART library on the Serial1 port of Arduino Mega
KnxTpUart knx(&Serial1, 15, 15, 20);

// Define input pin
int inPin = 32;

// Remember if we have sent on current keypress
boolean haveSent = false;

// Remember if we sent ON last or OFF
boolean onSent = false;


void setup() {
  pinMode(inPin, INPUT);
  digitalWrite(inPin, HIGH); // Turn on pullup
  
  // LED
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  
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
   if (digitalRead(inPin) == LOW) {
       // Button is pressed
       digitalWrite(13, HIGH);
       
       if (!haveSent) {
           // Send the opposite of what we have sent last
           knx.groupWriteBool(0,0,3, !onSent);
           onSent = !onSent;
           haveSent = true;
       }
   } else {
       digitalWrite(13, LOW); 
       haveSent = false;
   }
}

void serialEvent1() {
  KnxTpUartSerialEventType eType = knx.serialEvent();
  if (eType == KNX_TELEGRAM) {
     Serial.println("Event KNX_TELEGRAM");
  }
}







