#include "KnxTpUart.h"

KnxTpUart::KnxTpUart(HardwareSerial* sport, int area, int line, int member) {
	_serialport = sport;
	_source_area = area;
	_source_line = line;
	_source_member = member;
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


boolean KnxTpUart::isKNXControlByte(int b) {
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
	int buf[23];
	buf[0] = serialRead();
	
	// Parse Repeat Flag
	if (buf[0] & B00100000) {
		_tg.repeated = false; 
	} else {
		_tg.repeated = true; 
	}
	
	// Priority
	_tg.priority = ((buf[0] & B00001100) >> 2);
	
	// Source Address
	buf[1] = serialRead();
	buf[2] = serialRead();
	
	_tg.source_area = (buf[1] >> 4);
	_tg.source_line = (buf[1] & B00001111);
	_tg.source_member = buf[2];
	
	// Target Address
	buf[3] = serialRead();
	buf[4] = serialRead();
	buf[5] = serialRead();
	
	_tg.targetIsGroup = buf[5] & B10000000;
	if (_tg.targetIsGroup) {
		_tg.target_main_group = ((buf[3] & B01111000) >> 3);
		_tg.target_middle_group = (buf[3] & B00000111);
		_tg.target_sub_group = buf[4];
	}
	
	// Routing Counter
	_tg.routingcounter = ((buf[5] & B01110000) >> 4);
	
	// Payload Length
	_tg.payload_length = (buf[5] & B00001111);
	
	int bufpos = 6;
	for (int i = 0; i <= _tg.payload_length; i++) {
		buf[bufpos] = serialRead();
		bufpos++; 
	}
	
	_tg.command = ((buf[6] & B00000011) << 2) | ((buf[7] & B11000000) >> 6);
	
	_tg.firstDataByte = (buf[7] & B00111111);
	
	// Checksum
	buf[bufpos] = serialRead();
	_tg.checksum = buf[bufpos];

	// Test
	sendAck();
	
	
	
	if (TPUART_DEBUG) {
		Serial.print("Repeated: ");
		Serial.println(_tg.repeated);
		
		Serial.print("Priority: ");
		Serial.println(_tg.priority);

		Serial.print("Source: ");
		Serial.print(_tg.source_area);
		Serial.print(".");
		Serial.print(_tg.source_line);
		Serial.print(".");
		Serial.println(_tg.source_member);
		
		Serial.print("Target Group: ");
		Serial.print(_tg.target_main_group);
		Serial.print("/");
		Serial.print(_tg.target_middle_group);
		Serial.print("/");
		Serial.println(_tg.target_sub_group);
		
		Serial.print("Routing Counter: ");
		Serial.println(_tg.routingcounter);

		Serial.print("Payload Length: ");
		Serial.println(_tg.payload_length);

		Serial.print("Command: ");
		Serial.println(_tg.command);

		Serial.print("First Data Byte: ");
		Serial.println(_tg.firstDataByte);
	}
	
	
	
	int calculatedChecksum = calculateTelegramChecksum(buf, bufpos);
	
	if (_tg.checksum == calculatedChecksum) {
		Serial.println("Checksum matches");
	} else {
		Serial.println("Checksum mismatch");
		Serial.println(_tg.checksum, BIN);
		Serial.println(calculatedChecksum, BIN);
	}
	
	
}

KNXTelegram* KnxTpUart::getReceivedTelegram() {
	return &_tg;
}

void KnxTpUart::groupWriteBoolean(int mainGroup, int middleGroup, int subGroup, boolean value) {
	int valueAsInt = 0;
	if (value) {
		valueAsInt = B00000001;
	}
	
	int* buf = createKNXMessageFrame(2, KNX_COMMAND_WRITE, mainGroup, middleGroup, subGroup, valueAsInt);
	int messageSize = KNX_FRAME_SIZE + 2;
	sendMessage(buf, messageSize);	
}

void KnxTpUart::groupAnswerBoolean(int mainGroup, int middleGroup, int subGroup, boolean value) {
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
	buf[messageSize - 1] = calculateTelegramChecksum(buf, messageSize - 1);
	
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

int KnxTpUart::calculateTelegramChecksum(int* buf, int size) {
	int bcc = 0xFF;
	
	for (int i = 0; i < size; i++) {
		bcc ^= buf[i];
	}
	
	return bcc;
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