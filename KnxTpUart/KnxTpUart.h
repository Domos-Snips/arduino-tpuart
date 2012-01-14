
#ifndef KnxTpUart_h
#define KnxTpUart_h

#include "HardwareSerial.h"
#include "Arduino.h"

// Services from TPUART
#define TPUART_RESET_INDICATION_BYTE B11

// Services to TPUART
#define TPUART_DATA_START_CONTINUE B10000000
#define TPUART_DATA_END B01000000



#define TPUART_DEBUG true

#define KNX_COMMAND_READ B0000
#define KNX_COMMAND_WRITE B0010
#define KNX_COMMAND_ANSWER B0001

enum KnxTpUartSerialEventType {
	TPUART_RESET_INDICATION,
	TPUART_STATE_INDICATION,
	KNX_TELEGRAM,
	UNKNOWN
};

struct KNXTelegram {
    boolean repeated;
    int priority;
    int source_area;
    int source_line;
    int source_member;
    boolean targetIsGroup;
    int target_main_group;
    int target_middle_group;
    int target_sub_group;
    int routingcounter;
    int payload_length;
    int command;
    int firstDataByte;
    int checksum;
};


class KnxTpUart {
public:
	KnxTpUart(HardwareSerial*);
	void uartReset();
	void uartStateRequest();
	KnxTpUartSerialEventType serialEvent();
	KNXTelegram* getReceivedTelegram();
	
	void groupWriteBoolean(int, int, int, boolean);
private:
	HardwareSerial* _serialport;
	KNXTelegram _tg;
	
	boolean isKNXControlByte(int);
	void checkErrors();
	void printByte(int);
	void readKNXTelegram();
	int calculateTelegramChecksum(int*, int);
};



#endif