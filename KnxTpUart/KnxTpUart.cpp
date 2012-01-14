#include "KnxTpUart.h"


KnxTpUart::KnxTpUart(HardwareSerial* sport) {
	_serialport = sport;
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
	while (Serial1.available() > 0) {
		checkErrors();
		
		int incomingByte = Serial1.peek();
		printByte(incomingByte);
		
		if (isKNXControlByte(incomingByte)) {
			if (TPUART_DEBUG) Serial.println("KNX Control Byte received");
			readKNXTelegram();
			return KNX_TELEGRAM;
		} else if (incomingByte == TPUART_RESET_INDICATION_BYTE) {
			_serialport->read();
			return TPUART_RESET_INDICATION;
		} else if (incomingByte & B00000111) {
			_serialport->read();
			return TPUART_STATE_INDICATION;
		} else {
			_serialport->read();
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
	buf[0] = Serial1.read();
	
	// Parse Repeat Flag
	if (buf[0] & B00100000) {
		_tg.repeated = false; 
	} else {
		_tg.repeated = true; 
	}
	Serial.print("Repeated: ");
	Serial.println(_tg.repeated);
	
	// Priority
	_tg.priority = ((buf[0] & B00001100) >> 2);
	Serial.print("Priority: ");
	Serial.println(_tg.priority);
	
	// Source Address
	buf[1] = Serial1.read();
	buf[2] = Serial1.read();
	
	_tg.source_area = (buf[1] >> 4);
	_tg.source_line = (buf[1] & B00001111);
	_tg.source_member = buf[2];
	
	Serial.print("Source: ");
	Serial.print(_tg.source_area);
	Serial.print(".");
	Serial.print(_tg.source_line);
	Serial.print(".");
	Serial.println(_tg.source_member);
	
	// Target Address
	buf[3] = Serial1.read();
	buf[4] = Serial1.read();
	buf[5] = Serial1.read();
	
	_tg.targetIsGroup = buf[5] & B10000000;
	if (_tg.targetIsGroup) {
		_tg.target_main_group = ((buf[3] & B01111000) >> 3);
		_tg.target_middle_group = (buf[3] & B00000111);
		_tg.target_sub_group = buf[4];
		
		Serial.print("Target Group: ");
		Serial.print(_tg.target_main_group);
		Serial.print("/");
		Serial.print(_tg.target_middle_group);
		Serial.print("/");
		Serial.println(_tg.target_sub_group);
	}
	
	// Routing Counter
	_tg.routingcounter = ((buf[5] & B01110000) >> 4);
	Serial.print("Routing Counter: ");
	Serial.println(_tg.routingcounter);
	
	// Payload Length
	_tg.payload_length = (buf[5] & B00001111);
	Serial.print("Payload Length: ");
	Serial.println(_tg.payload_length);
	
	int bufpos = 6;
	for (int i = 0; i <= _tg.payload_length; i++) {
		buf[bufpos] = Serial1.read();
		bufpos++; 
	}
	
	Serial.print("Next buffer position: ");
	Serial.println(bufpos);
	
	_tg.command = ((buf[6] & B00000011) << 2) | ((buf[7] & B11000000) >> 6);
	Serial.print("Command: ");
	Serial.println(_tg.command);
	
	_tg.firstDataByte = (buf[7] & B00111111);
	Serial.print("First Data Byte: ");
	Serial.println(_tg.firstDataByte);
	
	// Checksum
	buf[bufpos] = Serial1.read();
	_tg.checksum = buf[bufpos];
	
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
	int buf[9];
	int messageSize = 9;
	
	uint8_t sendbuf[2];
	
	int valueAsInt = 0;
	if (value) {
		valueAsInt = B00000001;
	}
	
	int source_area = 15;
	int source_line = 15;
	int source_member = 20;
	
	buf[0] = B10111100; // Control Field, Normal Priority, No Repeat
	buf[1] = (source_area << 4) | source_line;	// Source Address
	buf[2] = source_member; // Source Address
	buf[3] = (mainGroup << 3) | middleGroup;
	buf[4] = subGroup;
	buf[5] = B11100001; // Target Group Address, Routing Counter = 6, Length = 1 (= 2 Bytes)
	buf[6] = KNX_COMMAND_WRITE >> 2; // Command
	buf[7] = (KNX_COMMAND_WRITE << 6) | valueAsInt; // Command and first data
	buf[8] = calculateTelegramChecksum(buf, 8);
	
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
}


int KnxTpUart::calculateTelegramChecksum(int* buf, int size) {
	int bcc = 0xFF;
	
	for (int i = 0; i < size; i++) {
		bcc ^= buf[i];
	}
	
	return bcc;
}