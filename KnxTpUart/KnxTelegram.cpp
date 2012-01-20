#include "KnxTelegram.h"

KnxTelegram::KnxTelegram() {
	clear();
}

void KnxTelegram::clear() {
	for (int i = 0; i < MAX_KNX_TELEGRAM_SIZE; i++) {
		buffer[i] = 0;
	}

	// Control Field, Normal Priority, No Repeat
	buffer[0] = B10111100;

	// Target Group Address, Routing Counter = 6, Length = 1 (= 2 Bytes)
	buffer[5] = B11100001;
}

int KnxTelegram::getBufferByte(int index) {
	return buffer[index];
}

void KnxTelegram::setBufferByte(int index, int content) {
	buffer[index] = content;
}

bool KnxTelegram::isRepeated() {
	// Parse Repeat Flag
	if (buffer[0] & B00100000) {
		return false;
	} else {
		return true;
	}
}

void KnxTelegram::setRepeated(bool repeat) {
	if (repeat) {
		buffer[0] = buffer[0] & B11011111;
	} else {
		buffer[0] = buffer[0] | B00100000;
	}
}

void KnxTelegram::setPriority(KnxPriorityType prio) {
	buffer[0] = buffer[0] & B11110011;
	buffer[0] = buffer[0] | (prio << 2);
}

KnxPriorityType KnxTelegram::getPriority() {
	// Priority
	return (KnxPriorityType) ((buffer[0] & B00001100) >> 2);
}

void KnxTelegram::setSourceAddress(int area, int line, int member) {
	buffer[1] = (area << 4) | line;	// Source Address
	buffer[2] = member; // Source Address
}

int KnxTelegram::getSourceArea() {
	return (buffer[1] >> 4);
}

int KnxTelegram::getSourceLine() {
	return (buffer[1] & B00001111);
}

int KnxTelegram::getSourceMember() {
	return buffer[2];
}

void KnxTelegram::setTargetGroupAddress(int main, int middle, int sub) {
	buffer[3] = (main << 3) | middle;
	buffer[4] = sub;
	buffer[5] = buffer[5] | B10000000;
}

bool KnxTelegram::isTargetGroup() {
	return buffer[5] & B10000000;
}

int KnxTelegram::getTargetMainGroup() {
	return ((buffer[3] & B01111000) >> 3);
}

int KnxTelegram::getTargetMiddleGroup() {
	return (buffer[3] & B00000111);
}

int KnxTelegram::getTargetSubGroup() {
	return buffer[4];
}

void KnxTelegram::setRoutingCounter(int counter) {
	buffer[5] = buffer[5] & B10000000;
	buffer[5] = buffer[5] | (counter << 4);
}

int KnxTelegram::getRoutingCounter() {
	return ((buffer[5] & B01110000) >> 4);
}

void KnxTelegram::setPayloadLength(int length) {
	buffer[5] = buffer[5] & B11110000;
	buffer[5] = buffer[5] | (length - 1);
}

int KnxTelegram::getPayloadLength() {
	return (buffer[5] & B00001111) + 1;
}

void KnxTelegram::setCommand(KnxCommandType command) {
	buffer[6] = buffer[6] & B11111100;
	buffer[7] = buffer[7] & B00111111;

	buffer[6] = buffer[6] | (command >> 2); // Command first two bits
	buffer[7] = buffer[7] | (command << 6); // Command last two bits
}

KnxCommandType KnxTelegram::getCommand() {
	return (KnxCommandType) (((buffer[6] & B00000011) << 2) | ((buffer[7] & B11000000) >> 6));
}

void KnxTelegram::setFirstDataByte(int data) {
	buffer[7] = buffer[7] & B11000000;
	buffer[7] = buffer[7] | data;
}

int KnxTelegram::getFirstDataByte() {
	return (buffer[7] & B00111111);
}

void KnxTelegram::createChecksum() {
	int checksumPos = getPayloadLength() + KNX_TELEGRAM_HEADER_SIZE;
	buffer[checksumPos] = calculateChecksum();
}

int KnxTelegram::getChecksum() {
	int checksumPos = getPayloadLength() + KNX_TELEGRAM_HEADER_SIZE;
	return buffer[checksumPos];
}

bool KnxTelegram::verifyChecksum() {
	int calculatedChecksum = calculateChecksum();
	return (getChecksum() == calculatedChecksum);
}

void KnxTelegram::print(HardwareSerial* serial) {
	serial->print("Repeated: ");
	serial->println(isRepeated());

	serial->print("Priority: ");
	serial->println(getPriority());

	serial->print("Source: ");
	serial->print(getSourceArea());
	serial->print(".");
	serial->print(getSourceLine());
	serial->print(".");
	serial->println(getSourceMember());

	serial->print("Target Group: ");
	serial->print(getTargetMainGroup());
	serial->print("/");
	serial->print(getTargetMiddleGroup());
	serial->print("/");
	serial->println(getTargetSubGroup());

	serial->print("Routing Counter: ");
	serial->println(getRoutingCounter());

	serial->print("Payload Length: ");
	serial->println(getPayloadLength());

	serial->print("Command: ");
	serial->println(getCommand());

	serial->print("First Data Byte: ");
	serial->println(getFirstDataByte());

	if (verifyChecksum()) {
		serial->println("Checksum matches");
	} else {
		serial->println("Checksum mismatch");
		serial->println(getChecksum(), BIN);
		serial->println(calculateChecksum(), BIN);
	}

}

int KnxTelegram::calculateChecksum() {
	int bcc = 0xFF;
	int size = getPayloadLength() + KNX_TELEGRAM_HEADER_SIZE;

	for (int i = 0; i < size; i++) {
		bcc ^= buffer[i];
	}

	return bcc;
}

int KnxTelegram::getTotalLength() {
	return KNX_TELEGRAM_HEADER_SIZE + getPayloadLength() + 1;
}
