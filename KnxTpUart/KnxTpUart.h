
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

// Size of KNX message without payload (header + checksum byte)
#define KNX_FRAME_SIZE 7

#define SERIAL_READ_TIMEOUT_MS 50

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
	
	void groupWriteBool(int, int, int, bool);
	
	void groupAnswerBool(int, int, int, bool);
private:
	HardwareSerial* _serialport;
	KnxTelegram* _tg;
	int _source_area;
	int _source_line;
	int _source_member;
	
	
	bool isKNXControlByte(int);
	void checkErrors();
	void printByte(int);
	void readKNXTelegram();
	void createKNXMessageFrame(int, KnxCommandType, int, int, int, int);
	void sendMessage();
	int serialRead();
};



#endif
