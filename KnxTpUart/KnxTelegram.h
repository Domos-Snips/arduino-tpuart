#ifndef KnxTelegram_h
#define KnxTelegram_h

#include "Arduino.h"

#define MAX_KNX_TELEGRAM_SIZE 23
#define KNX_TELEGRAM_HEADER_SIZE 6

class KnxTelegram {
	public:
		KnxTelegram();
		
		void clear();
		void setBufferByte(int index, int content);
		int getBufferByte(int index);
		void setPayloadLength(int size);
		int getPayloadLength();
		bool isRepeated();
		int getPriority();
		int getSourceArea();
		int getSourceLine();
		int getSourceMember();
		bool isTargetGroup();
		int getTargetMainGroup();
		int getTargetMiddleGroup();
		int getTargetSubGroup();
		int getRoutingCounter();
		int getCommand();
		int getFirstDataByte();
		bool verifyChecksum();
		int getChecksum();
		void print(HardwareSerial*);
	private:
		int buffer[MAX_KNX_TELEGRAM_SIZE];
		int calculateChecksum();

};

#endif

