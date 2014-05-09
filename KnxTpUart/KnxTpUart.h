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
// uncomment the following line to enable debugging
//#define TPUART_DEBUG true
#define TPUART_DEBUG_PORT Serial

#define TPUART_SERIAL_CLASS Stream

// Delay in ms between sending of packets to the bus
// set to 0 if you keep care of that by yourself
#define SERIAL_WRITE_DELAY_MS 0

// Timeout for reading a byte from TPUART
#define SERIAL_READ_TIMEOUT_MS 10

// Maximum number of group addresses that can be listened on
#define MAX_LISTEN_GROUP_ADDRESSES 15

enum KnxTpUartSerialEventType {
	TPUART_RESET_INDICATION,
	KNX_TELEGRAM,
	IRRELEVANT_KNX_TELEGRAM,
	UNKNOWN
};

class KnxTpUart {
public:
	KnxTpUart(TPUART_SERIAL_CLASS*, String);
	void uartReset();
	void uartStateRequest();
	KnxTpUartSerialEventType serialEvent();
	KnxTelegram* getReceivedTelegram();
	
    void setIndividualAddress(int, int, int);
    
	void sendAck();
	void sendNotAddressed();
	
	bool groupWriteBool(String, bool);
	bool groupWrite2ByteFloat(String, float);
	bool groupWrite1ByteInt(String, int);
	bool groupWrite2ByteInt(String, int);
	bool groupWrite4ByteFloat(String, float);
    bool groupWrite14ByteText(String, String);

	bool groupAnswerBool(String, bool);
	bool groupAnswer2ByteFloat(String, float);
	bool groupAnswer1ByteInt(String, int);
	bool groupAnswer2ByteInt(String, int);
	bool groupAnswer4ByteFloat(String, float);
    bool groupAnswer14ByteText(String, String);
	
	bool groupWriteTime(String, int, int, int, int);

	void addListenGroupAddress(String);
	bool isListeningToGroupAddress(int, int, int);
    
    bool individualAnswerAddress();
    bool individualAnswerMaskVersion(int, int, int);
    bool individualAnswerAuth(int, int, int, int, int);
    
    void setListenToBroadcasts(bool);
    
    
private:
	Stream* _serialport;
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
	void createKNXMessageFrame(int, KnxCommandType, String, int);
	void createKNXMessageFrameIndividual(int, KnxCommandType, String, int);
	bool sendMessage();
    bool sendNCDPosConfirm(int, int, int, int);
	int serialRead();
};



#endif
