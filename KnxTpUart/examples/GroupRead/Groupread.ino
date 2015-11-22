/*
   Beispiel für Leseanfrage ARDUINO UNO -> KNX und Auswertung Antwort KNX -> ARDUINO UNO
  
   Version 0.2
  
   22.11.2015
  
   Testkonstellation = ARDUINO UNO <-> 5WG1 117-2AB12
*/


#include <KnxTpUart.h>

KnxTpUart knx(&Serial, "1.1.199");
int LED = 13;
int target_2_6_0;
int target_5_6_0;


void setup() {

  pinMode(LED, OUTPUT); // PIN 13 als Ausgang festlegen
 
  Serial.begin(19200);
  UCSR0C = UCSR0C | B00100000; // Gerade Parität

  knx.uartReset();
 
  knx.addListenGroupAddress("2/6/0");
  knx.addListenGroupAddress("5/6/0");
 
  // Leseanfrage auf Gruppenadresse -> alle Datentypen
 
  // Die Funktion delay(1000) nur zur Verzögerung der Initialisierungsabfrage notwendig.
  // Wenn man keine Initialisierungsabfrage möchte, sollte man die nächsten Zeilen bis Ende der Funktion aus kommentieren.
 
  delay(1000);
 
  // Leseanfrage auf Gruppenadressen -> Aufruf der Funktion Leseanfrage auch in void loop() möglich
 
  knx.groupRead("2/6/0");
  knx.groupRead("5/6/0");
}


void loop() {
  
}


void serialEvent() {
 
  //Aufruf knx.serialEvent()
 
  KnxTpUartSerialEventType eType = knx.serialEvent();

  //Auswertung des empfangenen Telegrammes -> nur KNX-Telegramme werden akzeptiert
 
  if (eType == KNX_TELEGRAM) {
    KnxTelegram* telegram = knx.getReceivedTelegram();

    // Telegrammauswertung auf KNX (bei Empfang immer notwendig)
  
    String target =
      String(0 + telegram->getTargetMainGroup())   + "/" +
      String(0 + telegram->getTargetMiddleGroup()) + "/" +
      String(0 + telegram->getTargetSubGroup());

    // Auswertung der Gruppenadresse des empfangenen Telegrammes und Zwischenspeicherung in Variable "target"
  
    if (telegram->getCommand() == KNX_COMMAND_ANSWER) {
    
      // Auswertung der Leseanfrage in serialEvent() mit Datentypen
      // Auswertung des empfangenen KNX-Telegrammes mit Antwort auf Leseanfrage (Flag) -> Aktion
    
      if (target == "2/6/0") {
        target_2_6_0 = telegram->getBool();
      
        // Speicherung des Inhaltes in Variable "target_5_6_0" der Antwort auf die Leseanfrage der Gruppenadresse "2/6/0"
      
        if (target_2_6_0) {
          digitalWrite(LED, HIGH);
        }
        else {
          digitalWrite(LED,LOW);
        }
      
        // Auswertung des Inhaltes und Ausgabe des Inhaltes der Gruppenadresse "2/6/0" auf PIN 13 des ARDUINO UNO
      }
      else if (target == "5/6/0") {
        target_5_6_0 = telegram->get1ByteIntValue();
      
        // Speicherung des Inhaltes in Variable "target_5_6_0" der Antwort auf die Leseanfrage der Gruppenadresse "5/6/0"
      
      }
    }
  }
}