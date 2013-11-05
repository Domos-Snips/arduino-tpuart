
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
#define TPUART_DEBUG_PORT SerialUSB

#define TPUART_SERIAL_CLASS USARTClass

// Timeout for reading a byte from TPUART
#define SERIAL_READ_TIMEOUT_MS 10

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
	KnxTpUart(TPUART_SERIAL_CLASS*, int, int, int);
	void uartReset();
	void uartStateRequest();
	KnxTpUartSerialEventType serialEvent();
	KnxTelegram* getReceivedTelegram();
	
    void setIndividualAddress(int, int, int);
    
	void sendAck();
	void sendNotAddressed();
	
	bool groupWriteBool(int, int, int, bool);
	bool groupWrite2ByteFloat(int, int, int, float);
	
	bool groupAnswerBool(int, int, int, bool);
	bool groupAnswer2ByteFloat(int, int, int, float);

	void addListenGroupAddress(int, int, int);
	bool isListeningToGroupAddress(int, int, int);
    
    bool individualAnswerAddress();
    bool individualAnswerMaskVersion(int, int, int);
    bool individualAnswerAuth(int, int, int, int, int);
    
    void setListenToBroadcasts(bool);
    
    
private:
	HardwareSerial* _serialport;
	KnxTelegram* _tg;       // for normal communication
    KnxTelegram* _tg_ptp;   // for PTP sequence confirmation
	int _source_area;
	int _source_line;
	int _source_member;
	int _listen_group_addresses[MAX_LISTEN_GROUP_ADDRESSES][3];
	int _listen_group_address_count;
    bool _listen_to_broadcasts;
	
	bool isKNXControlByte(int);
	void checkErrors();
	void printByte(int);
	bool readKNXTelegram();
	void createKNXMessageFrame(int, KnxCommandType, int, int, int, int);
	void createKNXMessageFrameIndividual(int, KnxCommandType, int, int, int, int);
	bool sendMessage();
    bool sendNCDPosConfirm(int, int, int, int);
	int serialRead();
};



#endif
