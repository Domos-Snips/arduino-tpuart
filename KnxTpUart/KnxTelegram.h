#ifndef KnxTelegram_h
#define KnxTelegram_h

#include "Arduino.h"

#define MAX_KNX_TELEGRAM_SIZE 23
#define KNX_TELEGRAM_HEADER_SIZE 6

// KNX priorities
enum KnxPriorityType {
	KNX_PRIORITY_SYSTEM = B00,
	KNX_PRIORITY_ALARM = B10,
	KNX_PRIORITY_HIGH = B01,
	KNX_PRIORITY_NORMAL = B11
};

// KNX commands
enum KnxCommandType {
	KNX_COMMAND_READ = B0000,
	KNX_COMMAND_WRITE = B0010,
	KNX_COMMAND_ANSWER = B0001
};


class KnxTelegram {
	public:
		KnxTelegram();
		
		void clear();
		void setBufferByte(int index, int content);
		int getBufferByte(int index);
		void setPayloadLength(int size);
		int getPayloadLength();
		void setRepeated(bool repeat);
		bool isRepeated();
		void setPriority(KnxPriorityType prio);
		KnxPriorityType getPriority();
		void setSourceAddress(int area, int line, int member);
		int getSourceArea();
		int getSourceLine();
		int getSourceMember();
		void setTargetGroupAddress(int main, int middle, int sub);
		bool isTargetGroup();
		int getTargetMainGroup();
		int getTargetMiddleGroup();
		int getTargetSubGroup();
		void setRoutingCounter(int counter);
		int getRoutingCounter();
		void setCommand(KnxCommandType command);
		KnxCommandType getCommand();
		void setFirstDataByte(int data);
		int getFirstDataByte();
		void createChecksum();
		bool verifyChecksum();
		int getChecksum();
		void print(HardwareSerial*);
	private:
		int buffer[MAX_KNX_TELEGRAM_SIZE];
		int calculateChecksum();

};

#endif

