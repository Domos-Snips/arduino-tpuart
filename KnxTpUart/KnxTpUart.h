
#ifndef KnxTpUart_h
#define KnxTpUart_h

#include "HardwareSerial.h"
#include "Arduino.h"

#include "KnxTelegram.h"

// Services from TPUART
#define TPUART_RESET_INDICATION_BYTE B11

// Services to TPUART
#define TPUART_DATA_START_CONTINUE B10000000
#define TPUART_DATA_END B01000000

// Debugging
#define TPUART_DEBUG true

// Timeout for reading a byte from TPUART
#define SERIAL_READ_TIMEOUT_MS 50

// Maximum number of group addresses that can be listened on
#define MAX_LISTEN_GROUP_ADDRESSES 5

enum KnxTpUartSerialEventType {
	TPUART_RESET_INDICATION,
	KNX_TELEGRAM,
	IRRELEVANT_KNX_TELEGRAM,
	UNKNOWN
};

class KnxTpUart {
public:
	KnxTpUart(HardwareSerial*, int, int, int);
	void uartReset();
	void uartStateRequest();
	KnxTpUartSerialEventType serialEvent();
	KnxTelegram* getReceivedTelegram();
	
	void sendAck();
	void sendNotAddressed();
	
	void groupWriteBool(int, int, int, bool);
	
	void groupAnswerBool(int, int, int, bool);

	void addListenGroupAddress(int, int, int);
	bool isListeningToGroupAddress(int, int, int);
private:
	HardwareSerial* _serialport;
	KnxTelegram* _tg;
	int _source_area;
	int _source_line;
	int _source_member;
	int _listen_group_addresses[MAX_LISTEN_GROUP_ADDRESSES][3];
	int _listen_group_address_count;
	
	bool isKNXControlByte(int);
	void checkErrors();
	void printByte(int);
	bool readKNXTelegram();
	void createKNXMessageFrame(int, KnxCommandType, int, int, int, int);
	void sendMessage();
	int serialRead();
};



#endif
