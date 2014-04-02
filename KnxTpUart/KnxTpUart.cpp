#include "KnxTpUart.h"

KnxTpUart::KnxTpUart(TPUART_SERIAL_CLASS* sport, int area, int line, int member) {
	_serialport = sport;
	_source_area = area;
	_source_line = line;
	_source_member = member;
	_listen_group_address_count = 0;
	_tg = new KnxTelegram();
	_tg_ptp = new KnxTelegram();
    _listen_to_broadcasts = false;
}

void KnxTpUart::setListenToBroadcasts(bool listen) {
    _listen_to_broadcasts = listen;
}

void KnxTpUart::uartReset() {
	byte sendByte = 0x01;
	_serialport->write(sendByte);
}

void KnxTpUart::uartStateRequest() {
	byte sendByte = 0x02;
	_serialport->write(sendByte);
}

void KnxTpUart::setIndividualAddress(int area, int line, int member) {
    _source_area = area;
    _source_line = line;
    _source_member = member;
}

KnxTpUartSerialEventType KnxTpUart::serialEvent() {
	while (_serialport->available() > 0) {
		checkErrors();
		
		int incomingByte = _serialport->peek();
		printByte(incomingByte);
		
		if (isKNXControlByte(incomingByte)) {
			bool interested = readKNXTelegram();
			if (interested) {
				if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Event KNX_TELEGRAM");
				return KNX_TELEGRAM;
			} else {
				if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Event IRRELEVANT_KNX_TELEGRAM");
				return IRRELEVANT_KNX_TELEGRAM;
			}
		} else if (incomingByte == TPUART_RESET_INDICATION_BYTE) {
			serialRead();
			if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Event TPUART_RESET_INDICATION");
			return TPUART_RESET_INDICATION;
		} else {
			serialRead();
			if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Event UNKNOWN");
			return UNKNOWN;
		}
	}

	if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Event UNKNOWN");
	return UNKNOWN;
}


bool KnxTpUart::isKNXControlByte(int b) {
	return ( (b | B00101100) == B10111100 ); // Ignore repeat flag and priority flag
}

void KnxTpUart::checkErrors() {

#if defined(_SAM3XA_)  // For DUE
	if (USART1->US_CSR & US_CSR_OVRE) {
		if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Overrun"); 
	}

	if (USART1->US_CSR & US_CSR_FRAME) {
		if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Frame Error");
	}

	if (USART1->US_CSR & US_CSR_PARE) {
		if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Parity Error");
	}
#elif defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) // for UNO
	if (UCSR0A & B00010000) {
		if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Frame Error"); 
	}
	
	if (UCSR0A & B00000100) {
		if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Parity Error"); 
	}
#else
	if (UCSR1A & B00010000) {
		if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Frame Error"); 
	}
	
	if (UCSR1A & B00000100) {
		if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Parity Error"); 
	}
#endif
}

void KnxTpUart::printByte(int incomingByte) {
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.print("Incoming Byte: ");
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.print(incomingByte, DEC);
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.print(" - ");
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.print(incomingByte, HEX);
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.print(" - ");
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.print(incomingByte, BIN);
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.println();
}

bool KnxTpUart::readKNXTelegram() {
	// Receive header
	for (int i = 0; i < 6; i++) {
		_tg->setBufferByte(i, serialRead());
	}

	if (TPUART_DEBUG) TPUART_DEBUG_PORT.print("Payload Length: ");
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.println(_tg->getPayloadLength());

	int bufpos = 6;
	for (int i = 0; i < _tg->getPayloadLength(); i++) {
		_tg->setBufferByte(bufpos, serialRead());
		bufpos++; 
	}

	// Checksum
	_tg->setBufferByte(bufpos, serialRead());

	// Print the received telegram
	if (TPUART_DEBUG) {
		_tg->print(&TPUART_DEBUG_PORT);
	}

    // Verify if we are interested in this message - GroupAddress
	bool interested = _tg->isTargetGroup() && isListeningToGroupAddress(_tg->getTargetMainGroup(), _tg->getTargetMiddleGroup(), _tg->getTargetSubGroup());
    
    // Physical address
    interested = interested || ((!_tg->isTargetGroup()) && _tg->getTargetArea() == _source_area && _tg->getTargetLine() == _source_line && _tg->getTargetMember() == _source_member);
    
    // Broadcast (Programming Mode)
    interested = interested || (_listen_to_broadcasts && _tg->isTargetGroup() && _tg->getTargetMainGroup() == 0 && _tg->getTargetMiddleGroup() == 0 && _tg->getTargetSubGroup() == 0);
    
	if (interested) {
		sendAck();
	} else {
		sendNotAddressed();
	}

    if (_tg->getCommunicationType() == KNX_COMM_UCD) {
        if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("UCD Telegram received");
    } else if (_tg->getCommunicationType() == KNX_COMM_NCD) {
        if (TPUART_DEBUG) {
            TPUART_DEBUG_PORT.print("NCD Telegram ");
            TPUART_DEBUG_PORT.print(_tg->getSequenceNumber());
            TPUART_DEBUG_PORT.println(" received");
        }
        
        sendNCDPosConfirm(_tg->getSequenceNumber(), _tg->getSourceArea(), _tg->getSourceLine(), _tg->getSourceMember());
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

bool KnxTpUart::groupWrite2ByteInt(int mainGroup, int middleGroup, int subGroup, int value) {
	createKNXMessageFrame(2, KNX_COMMAND_WRITE, mainGroup, middleGroup, subGroup, 0);
	_tg->set2ByteFloatValue(value);
	_tg->createChecksum();
	return sendMessage();
}

bool KnxTpUart::groupWrite1ByteInt(int mainGroup, int middleGroup, int subGroup, int value) {
	createKNXMessageFrame(2, KNX_COMMAND_WRITE, mainGroup, middleGroup, subGroup, 0);
	_tg->set1ByteIntValue(value);
	_tg->createChecksum();
	return sendMessage();
}

bool KnxTpUart::groupWrite4ByteFloat(int mainGroup, int middleGroup, int subGroup, float value) {
	createKNXMessageFrame(2, KNX_COMMAND_WRITE, mainGroup, middleGroup, subGroup, 0);
	_tg->set4ByteFloatValue(value);
	_tg->createChecksum();
	return sendMessage();
}

bool KnxTpUart::groupWrite14ByteText(int mainGroup, int middleGroup, int subGroup, String value) {
	createKNXMessageFrame(2, KNX_COMMAND_WRITE, mainGroup, middleGroup, subGroup, 0);
	_tg->set14ByteValue(value);
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

bool KnxTpUart::groupAnswer1ByteInt(int mainGroup, int middleGroup, int subGroup, int value) {
	createKNXMessageFrame(2, KNX_COMMAND_ANSWER, mainGroup, middleGroup, subGroup, 0);
	_tg->set1ByteIntValue(value);
	_tg->createChecksum();
	return sendMessage();
}

bool KnxTpUart::groupAnswer2ByteFloat(int mainGroup, int middleGroup, int subGroup, float value) {
	createKNXMessageFrame(2, KNX_COMMAND_ANSWER, mainGroup, middleGroup, subGroup, 0);
	_tg->set2ByteFloatValue(value);
	_tg->createChecksum();
	return sendMessage();
}

bool KnxTpUart::groupAnswer2ByteInt(int mainGroup, int middleGroup, int subGroup, int value) {
	createKNXMessageFrame(2, KNX_COMMAND_ANSWER, mainGroup, middleGroup, subGroup, 0);
	_tg->set2ByteFloatValue(value);
	_tg->createChecksum();
	return sendMessage();
}

bool KnxTpUart::groupAnswer4ByteFloat(int mainGroup, int middleGroup, int subGroup, float value) {
	createKNXMessageFrame(2, KNX_COMMAND_ANSWER, mainGroup, middleGroup, subGroup, 0);
	_tg->set4ByteFloatValue(value);
	_tg->createChecksum();
	return sendMessage();
}

bool KnxTpUart::groupAnswer14ByteText(int mainGroup, int middleGroup, int subGroup, String value) {
	createKNXMessageFrame(2, KNX_COMMAND_ANSWER, mainGroup, middleGroup, subGroup, 0);
	_tg->set14ByteValue(value);
	_tg->createChecksum();
	return sendMessage();
}

bool KnxTpUart::individualAnswerAddress() {
    createKNXMessageFrame(2, KNX_COMMAND_INDIVIDUAL_ADDR_RESPONSE, 0, 0, 0, 0);
	_tg->createChecksum();
	return sendMessage();    
}

bool KnxTpUart::individualAnswerMaskVersion(int area, int line, int member) {
    createKNXMessageFrameIndividual(4, KNX_COMMAND_MASK_VERSION_RESPONSE, area, line, member, 0);
    _tg->setCommunicationType(KNX_COMM_NDP);
    _tg->setBufferByte(8, 0x07); // Mask version part 1 for BIM M 112
    _tg->setBufferByte(9, 0x01); // Mask version part 2 for BIM M 112
    _tg->createChecksum();
    return sendMessage();
}

bool KnxTpUart::individualAnswerAuth(int accessLevel, int sequenceNo, int area, int line, int member) {
    createKNXMessageFrameIndividual(3, KNX_COMMAND_ESCAPE, area, line, member, KNX_EXT_COMMAND_AUTH_RESPONSE);
    _tg->setCommunicationType(KNX_COMM_NDP);
    _tg->setSequenceNumber(sequenceNo);
    _tg->setBufferByte(8, accessLevel);
    _tg->createChecksum();
    return sendMessage();
}

void KnxTpUart::createKNXMessageFrame(int payloadlength, KnxCommandType command, int mainGroup, int middleGroup, int subGroup, int firstDataByte) {
	_tg->clear();
	_tg->setSourceAddress(_source_area, _source_line, _source_member);
	_tg->setTargetGroupAddress(mainGroup, middleGroup, subGroup);
	_tg->setFirstDataByte(firstDataByte);
	_tg->setCommand(command);
    _tg->setPayloadLength(payloadlength);
	_tg->createChecksum();
}

void KnxTpUart::createKNXMessageFrameIndividual(int payloadlength, KnxCommandType command, int area, int line, int member, int firstDataByte) {
	_tg->clear();
	_tg->setSourceAddress(_source_area, _source_line, _source_member);
	_tg->setTargetIndividualAddress(area, line, member);
	_tg->setFirstDataByte(firstDataByte);
	_tg->setCommand(command);
    _tg->setPayloadLength(payloadlength);
	_tg->createChecksum();
}

bool KnxTpUart::sendNCDPosConfirm(int sequenceNo, int area, int line, int member) {
    _tg_ptp->clear();
	_tg_ptp->setSourceAddress(_source_area, _source_line, _source_member);
	_tg_ptp->setTargetIndividualAddress(area, line, member);
	_tg_ptp->setSequenceNumber(sequenceNo);
    _tg_ptp->setCommunicationType(KNX_COMM_NCD);
    _tg_ptp->setControlData(KNX_CONTROLDATA_POS_CONFIRM);
    _tg_ptp->setPayloadLength(1);
	_tg_ptp->createChecksum();
    
    
    int messageSize = _tg_ptp->getTotalLength();
    
	uint8_t sendbuf[2];
	for (int i = 0; i < messageSize; i++) {
		if (i == (messageSize - 1)) {
			sendbuf[0] = TPUART_DATA_END;
		} else {
			sendbuf[0] = TPUART_DATA_START_CONTINUE;
		}
		
		sendbuf[0] |= i;
		sendbuf[1] = _tg_ptp->getBufferByte(i);
		
		_serialport->write(sendbuf, 2);
	}
    
    
	int confirmation;
	while(true) {
		confirmation = serialRead();
		if (confirmation == B10001011) {
			return true; // Sent successfully
		} else if (confirmation == B00001011) {
			return false;
		} else if (confirmation == -1) {
			// Read timeout
			return false;
		}
	}
    
	return false;
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


	int confirmation;
	while(true) {
		confirmation = serialRead();
		if (confirmation == B10001011) {
			delay (SERIAL_WRITE_DELAY_MS);
			return true; // Sent successfully
		} else if (confirmation == B00001011) {
			delay (SERIAL_WRITE_DELAY_MS);
			return false;
		} else if (confirmation == -1) {
			// Read timeout
			delay (SERIAL_WRITE_DELAY_MS);
			return false;
		}
	}

	return false;
}

void KnxTpUart::sendAck() {
	byte sendByte = B00010001;
	_serialport->write(sendByte);
	delay(SERIAL_WRITE_DELAY_MS);
}

void KnxTpUart::sendNotAddressed() {
	byte sendByte = B00010000;
	_serialport->write(sendByte);
	delay(SERIAL_WRITE_DELAY_MS);
}

int KnxTpUart::serialRead() {
	unsigned long startTime = millis();
	
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.print("Available: ");
	if (TPUART_DEBUG) TPUART_DEBUG_PORT.println(_serialport->available());
	
	while (! (_serialport->available() > 0)) {
		if (abs(millis() - startTime) > SERIAL_READ_TIMEOUT_MS) {
			// Timeout
			if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Timeout while receiving message");
			return -1;
		}
		delay(1);
	}
	
	int inByte = _serialport->read();
	checkErrors();
	printByte(inByte);
	
	return inByte;
}

void KnxTpUart::addListenGroupAddress(int main, int middle, int sub) {
	if (_listen_group_address_count >= MAX_LISTEN_GROUP_ADDRESSES) {
		if (TPUART_DEBUG) TPUART_DEBUG_PORT.println("Already listening to MAX_LISTEN_GROUP_ADDRESSES, cannot listen to another");
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
