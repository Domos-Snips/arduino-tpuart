#include <KnxTpUart.h>
#include <ArduinoUnit.h>

TestSuite suite;
KnxTpUart knx(&Serial1, 15, 15, 20);
KnxTelegram* knxTelegram = new KnxTelegram();

void setup() {
}

test(knxTelegramClearAfterCreation) {
  knxTelegram = new KnxTelegram();

  for (int i = 0; i < MAX_KNX_TELEGRAM_SIZE; i++) {
    if (i != 0 && i != 5) {
     assertEquals(0, knxTelegram->getBufferByte(i));
    } 
  }  
  
  assertEquals(B10111100, knxTelegram->getBufferByte(0));
  assertEquals(B11100001, knxTelegram->getBufferByte(5));   
}

test(knxTelegramClear) {
  knxTelegram->setBufferByte(1, 12345);
  knxTelegram->clear();

  for (int i = 0; i < MAX_KNX_TELEGRAM_SIZE; i++) {
    if (i != 0 && i != 5) {
     assertEquals(0, knxTelegram->getBufferByte(i));
    } 
  }  
  
  assertEquals(B10111100, knxTelegram->getBufferByte(0));
  assertEquals(B11100001, knxTelegram->getBufferByte(5)); 
}

void loop() {
  suite.run();
}