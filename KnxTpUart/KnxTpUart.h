/*

   File: KnxTpUart.h

   Author: Daniel Kleine-Albers (Since 2012)
   Modified: Thorsten Gehrig (Since 2014)
   Modified: Michael Werski (Since 2014)
   Modified: Katja Blankenheim (Since 2014)
   Modified: Mag Gyver (Since 2016)

   Last modified: 05.08.2017
   Reason: Fixed error on initialization of serial port

*/

#ifndef KnxTpUart_h
#define KnxTpUart_h

#include "HardwareSerial.h"
#include "Arduino.h"

#include "KnxTelegram.h"

// Services from TPUART
#define TPUART_RESET_INDICATION_BYTE B00000011 // 0x03

/* Thanks to Mag Gyver for the help */

#define TPUART_DATA_CONFIRM_SUCCESS_BYTE B10001011 // 0x8B
#define TPUART_DATA_CONFIRM_FAILED_BYTE B00001011 // 0x0B

// Services to TPUART
#define TPUART_RESET_REQUEST B00000001 // 0x01
#define TPUART_STATE_REQUEST B00000010 // 0x02

/* Thanks to Mag Gyver for the help */

#define TPUART_SET_ADDRESS_REQUEST B00101000 // 0x28
#define TPUART_DATA_START_CONTINUE_REQUEST B10000000 // 0x80
#define TPUART_DATA_END_REQUEST B01000000 // 0x40
#define TPUART_ACK_SERVICE_ADDRESSED B00010001 // 0x11
#define TPUART_ACK_SERVICE_NOT_ADDRESSED B00010000 // 0x10

// Uncomment the following line to enable debugging
//#define TPUART_DEBUG

#define TPUART_DEBUG_PORT Serial

#define TPUART_SERIAL_CLASS Stream /* Thanks to Michael Werski for the help */

// Delay in ms between sending of packets to the bus
// Change only if you know what you're doing
#define SERIAL_WRITE_DELAY_MS 100 /* Thanks to Michael Werski for the help */

// Timeout for reading a byte from TPUART
// Change only if you know what you're doing
#define SERIAL_READ_TIMEOUT_MS 10

// Maximum number of group addresses that can be listened on
#define MAX_LISTEN_GROUP_ADDRESSES 16

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
    bool groupWrite4BitInt(String, int); /* Thanks to Mag Gyver for the help */
    bool groupWrite4BitDim(String, bool, byte); /* Thanks to Mag Gyver for the help */
    bool groupWrite1ByteInt(String, int); /* Thanks to Thorsten Gehrig for the help */
    bool groupWrite2ByteInt(String, int); /* Thanks to Thorsten Gehrig for the help */
    bool groupWrite2ByteFloat(String, float);
    bool groupWrite3ByteTime(String, int, int, int, int); /* Thanks to Thorsten Gehrig for the help */
    bool groupWrite3ByteDate(String, int, int, int); /* Thanks to Mag Gyver for the help */
    bool groupWrite4ByteFloat(String, float); /* Thanks to Thorsten Gehrig for the help */
    bool groupWrite14ByteText(String, String); /* Thanks to Thorsten Gehrig for the help */

    bool groupAnswerBool(String, bool);
    // Very rare to no need, but in case someone needs it
    /* Thanks to Mag Gyver for the help */
    /*
      bool groupAnswer4BitInt(String, int);
      bool groupAnswer4BitDim(String, bool, byte);
    */
    bool groupAnswer1ByteInt(String, int); /* Thanks to Thorsten Gehrig for the help */
    bool groupAnswer2ByteInt(String, int); /* Thanks to Thorsten Gehrig for the help */
    bool groupAnswer2ByteFloat(String, float);
    bool groupAnswer3ByteTime(String, int, int, int, int); /* Thanks to Thorsten Gehrig for the help */
    bool groupAnswer3ByteDate(String, int, int, int); /* Thanks to Mag Gyver for the help */
    bool groupAnswer4ByteFloat(String, float); /* Thanks to Thorsten Gehrig for the help */
    bool groupAnswer14ByteText(String, String); /* Thanks to Thorsten Gehrig for the help */

    bool groupRead(String); /* Thanks to Mag Gyver for the help */

    void addListenGroupAddress(String);
    bool isListeningToGroupAddress(int, int, int);

    bool individualAnswerAddress();
    bool individualAnswerMaskVersion(int, int, int);
    bool individualAnswerAuth(int, int, int, int, int);

    void setListenToBroadcasts(bool);


  private:
    Stream* _serialport; /* Thanks to Michael Werski for the help */
    KnxTelegram* _tg; // For normal communication
    KnxTelegram* _tg_ptp; // For PTP sequence confirmation
    int _source_area;
    int _source_line;
    int _source_member;
    int _confirmation_timeout; /* Thanks to Mag Gyver for the help */
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

/* End of KnxTpUart.h */