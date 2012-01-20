#include "KnxTpUart.h"

KnxTpUart::KnxTpUart(HardwareSerial* sport, int area, int line, int member) {
	_serialport = sport;
	_source_area = area;
	_source_line = line;
	_source_member = member;
	_listen_group_address_count = 0;
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
			bool interested = readKNXTelegram();
			if (interested) {
				if (TPUART_DEBUG) Serial.println("Event KNX_TELEGRAM");
				return KNX_TELEGRAM;
			} else {
				if (TPUART_DEBUG) Serial.println("Event IRRELEVANT_KNX_TELEGRAM");
				return IRRELEVANT_KNX_TELEGRAM;
			}
		} else if (incomingByte == TPUART_RESET_INDICATION_BYTE) {
			serialRead();
			if (TPUART_DEBUG) Serial.println("Event TPUART_RESET_INDICATION");
			return TPUART_RESET_INDICATION;
		} else {
			serialRead();
			if (TPUART_DEBUG) Serial.println("Event UNKNOWN");
			return UNKNOWN;
		}
	}

	if (TPUART_DEBUG) Serial.println("Event UNKNOWN");
	return UNKNOWN;
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

bool KnxTpUart::readKNXTelegram() {
	// Receive header
	for (int i = 0; i < 6; i++) {
		_tg->setBufferByte(i, serialRead());
	}

	// Verify if we are interested in this message
	bool interested = isListeningToGroupAddress(_tg->getTargetMainGroup(), _tg->getTargetMiddleGroup(), _tg->getTargetSubGroup());

	if (interested) {
		sendAck();
	} else {
		sendNotAddressed();
	}

	int bufpos = 6;
	for (int i = 0; i < _tg->getPayloadLength(); i++) {
		_tg->setBufferByte(bufpos, serialRead());
		bufpos++; 
	}
	
	// Checksum
	_tg->setBufferByte(bufpos, serialRead());
	
	// Print the received telegram
	if (TPUART_DEBUG) {
		_tg->print(&Serial);
	}
	
	// Returns if we are interested in this diagram
	return interested;
}

KnxTelegram* KnxTpUart::getReceivedTelegram() {
	return _tg;
}

bool KnxTpUart::groupWriteBool(int mainGroup, int middleGroup, int subGroup, bool value) {
	int valueAsInt = 0;
	if (value) {
		valueAsInt = B00000001;
	}
	
	createKNXMessageFrame(2, KNX_COMMAND_WRITE, mainGroup, middleGroup, subGroup, valueAsInt);
	return sendMessage();
}

bool KnxTpUart::groupWrite2ByteFloat(int mainGroup, int middleGroup, int subGroup, float value) {
	createKNXMessageFrame(2, KNX_COMMAND_WRITE, mainGroup, middleGroup, subGroup, 0);
	_tg->set2ByteFloatValue(value);
	_tg->createChecksum();
	return sendMessage();
}

bool KnxTpUart::groupAnswerBool(int mainGroup, int middleGroup, int subGroup, bool value) {
	int valueAsInt = 0;
	if (value) {
		valueAsInt = B00000001;
	}
	
	createKNXMessageFrame(2, KNX_COMMAND_ANSWER, mainGroup, middleGroup, subGroup, valueAsInt);
	return sendMessage();
}

bool KnxTpUart::groupAnswer2ByteFloat(int mainGroup, int middleGroup, int subGroup, float value) {
	createKNXMessageFrame(2, KNX_COMMAND_ANSWER, mainGroup, middleGroup, subGroup, 0);
	_tg->set2ByteFloatValue(value);
	_tg->createChecksum();
	return sendMessage();
}

void KnxTpUart::createKNXMessageFrame(int payloadlength, KnxCommandType command, int mainGroup, int middleGroup, int subGroup, int firstDataByte) {
	_tg->clear();
	_tg->setSourceAddress(_source_area, _source_line, _source_member);
	_tg->setTargetGroupAddress(mainGroup, middleGroup, subGroup);
	_tg->setFirstDataByte(firstDataByte);
	_tg->setCommand(command);
	_tg->createChecksum();
}

bool KnxTpUart::sendMessage() {
	int messageSize = _tg->getTotalLength();

	uint8_t sendbuf[2];
	for (int i = 0; i < messageSize; i++) {
		if (i == (messageSize - 1)) {
			sendbuf[0] = TPUART_DATA_END;
		} else {
			sendbuf[0] = TPUART_DATA_START_CONTINUE;
		}
		
		sendbuf[0] |= i;
		sendbuf[1] = _tg->getBufferByte(i);
		
		_serialport->write(sendbuf, 2);
	}

	// We'll receive the message immediately back from TP-UART
	for (int i = 0; i < messageSize; i++) {
		serialRead();
	}

	// Receive the confirmation byte
	int confirmation = serialRead();

	if (confirmation == B10001011) {
		return true; // Sent successfully
	} else {
		return false;
	}

}

void KnxTpUart::sendAck() {
	byte sendByte = B00010001;
	_serialport->write(sendByte);
}

void KnxTpUart::sendNotAddressed() {
	byte sendByte = B00010000;
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

void KnxTpUart::addListenGroupAddress(int main, int middle, int sub) {
	if (_listen_group_address_count >= MAX_LISTEN_GROUP_ADDRESSES) {
		if (TPUART_DEBUG) Serial.println("Already listening to MAX_LISTEN_GROUP_ADDRESSES, cannot listen to another");
		return;
	}

	_listen_group_addresses[_listen_group_address_count][0] = main;
	_listen_group_addresses[_listen_group_address_count][1] = middle;
	_listen_group_addresses[_listen_group_address_count][2] = sub;

	_listen_group_address_count++;
}

bool KnxTpUart::isListeningToGroupAddress(int main, int middle, int sub) {
	for (int i = 0; i < _listen_group_address_count; i++) {
		if ( (_listen_group_addresses[i][0] == main)
				&& (_listen_group_addresses[i][1] == middle)
				&& (_listen_group_addresses[i][2] == sub)) {
			return true;
		}
	}

	return false;
}
