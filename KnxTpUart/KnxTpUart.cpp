#include "KnxTpUart.h"

KnxTpUart::KnxTpUart(HardwareSerial* sport, int area, int line, int member) {
	_serialport = sport;
	_source_area = area;
	_source_line = line;
	_source_member = member;
	_tg = new KnxTelegram();
}

void KnxTpUart::uartReset() {
	byte sendByte = 0x01;
	_serialport->write(sendByte);
}

void KnxTpUart::uartStateRequest() {
	byte sendByte = 0x02;
	_serialport->write(sendByte);
}

KnxTpUartSerialEventType KnxTpUart::serialEvent() {
	while (_serialport->available() > 0) {
		checkErrors();
		
		int incomingByte = _serialport->peek();
		printByte(incomingByte);
		
		if (isKNXControlByte(incomingByte)) {
			if (TPUART_DEBUG) Serial.println("KNX Control Byte received");
			readKNXTelegram();
			return KNX_TELEGRAM;
		} else if (incomingByte == TPUART_RESET_INDICATION_BYTE) {
			serialRead();
			return TPUART_RESET_INDICATION;
		} else if (incomingByte & B00000111) {
			serialRead();
			return TPUART_STATE_INDICATION;
		} else {
			serialRead();
			return UNKNOWN;
		}
	}
}


bool KnxTpUart::isKNXControlByte(int b) {
	return ( (b | B00101100) == B10111100 ); // Ignore repeat flag and priority flag
}

void KnxTpUart::checkErrors() {
	if (UCSR1A & B00010000) {
		Serial.println("Frame Error"); 
	}
	
	if (UCSR1A & B00000100) {
		Serial.println("Parity Error"); 
	}
	
}

void KnxTpUart::printByte(int incomingByte) {
	Serial.print("Incoming Byte: ");
	Serial.print(incomingByte, HEX);
	Serial.print(" - ");
	Serial.print(incomingByte, BIN);
	Serial.println();
}

void KnxTpUart::readKNXTelegram() {
	// Receive header
	for (int i = 0; i < 6; i++) {
		_tg->setBufferByte(i, serialRead());
	}

	int bufpos = 6;
	for (int i = 0; i < _tg->getPayloadLength(); i++) {
		_tg->setBufferByte(bufpos, serialRead());
		bufpos++; 
	}
	
	// Checksum
	_tg->setBufferByte(bufpos, serialRead());

	// Test
	sendAck();
	
	// Print the received telegram
	if (TPUART_DEBUG) {
		_tg->print(&Serial);
	}
	
}

KnxTelegram* KnxTpUart::getReceivedTelegram() {
	return _tg;
}

void KnxTpUart::groupWriteBool(int mainGroup, int middleGroup, int subGroup, bool value) {
	int valueAsInt = 0;
	if (value) {
		valueAsInt = B00000001;
	}
	
	int* buf = createKNXMessageFrame(2, KNX_COMMAND_WRITE, mainGroup, middleGroup, subGroup, valueAsInt);
	int messageSize = KNX_FRAME_SIZE + 2;
	sendMessage(buf, messageSize);	
}

void KnxTpUart::groupAnswerBool(int mainGroup, int middleGroup, int subGroup, bool value) {
	int valueAsInt = 0;
	if (value) {
		valueAsInt = B00000001;
	}
	
	int* buf = createKNXMessageFrame(2, KNX_COMMAND_ANSWER, mainGroup, middleGroup, subGroup, valueAsInt);
	int messageSize = KNX_FRAME_SIZE + 2;
	sendMessage(buf, messageSize);	
}

int* KnxTpUart::createKNXMessageFrame(int payloadlength, int command, int mainGroup, int middleGroup, int subGroup, int firstDataByte) {
	int messageSize = KNX_FRAME_SIZE + payloadlength;
	int* buf = (int*) malloc(sizeof(int) * messageSize);
	
	buf[0] = B10111100; // Control Field, Normal Priority, No Repeat
	buf[1] = (_source_area << 4) | _source_line;	// Source Address
	buf[2] = _source_member; // Source Address
	buf[3] = (mainGroup << 3) | middleGroup;
	buf[4] = subGroup;
	buf[5] = B11100001; // Target Group Address, Routing Counter = 6, Length = 1 (= 2 Bytes)
	buf[6] = command >> 2; // Command
	buf[7] = (command << 6) | firstDataByte; // Command and first data
	//buf[messageSize - 1] = calculateTelegramChecksum(buf, messageSize - 1);
	
	return buf;
}

void KnxTpUart::sendMessage(int* buf, int messageSize) {
	uint8_t sendbuf[2];
	for (int i = 0; i < messageSize; i++) {
		if (i == (messageSize - 1)) {
			sendbuf[0] = TPUART_DATA_END;
		} else {
			sendbuf[0] = TPUART_DATA_START_CONTINUE;
		}
		
		sendbuf[0] |= i;
		sendbuf[1] = buf[i];
		
		_serialport->write(sendbuf, 2);
	}
	
	free(buf);
}

void KnxTpUart::sendAck() {
	byte sendByte = B00010001;
	_serialport->write(sendByte);
}

int KnxTpUart::serialRead() {
	unsigned long startTime = millis();
	
	while (! (_serialport->available() > 0)) {
		if (abs(millis() - startTime) > SERIAL_READ_TIMEOUT_MS) {
			// Timeout
			if (TPUART_DEBUG) Serial.println("Timeout while receiving message");
			return -1;
		}
		delay(1);
	}
	
	return _serialport->read();
}
