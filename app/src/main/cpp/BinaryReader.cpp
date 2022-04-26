#include "BinaryReader.h"

BinaryReader::BinaryReader(uint8_t *data, size_t size) {
    this->data = new uint8_t[size];
    memcpy(this->data, data, size);
    this->size = size;
    this->pos = 0;
}

BinaryReader::~BinaryReader() {
    free(this->data);
    this->size = 0;
    this->pos = 0;
}

bool BinaryReader::checkSize(size_t size) {
    if (this->pos + size > this->size) {
        return false;
    }
    return true;
}

int8_t BinaryReader::readInt8() {
    if (!this->checkSize(sizeof(int8_t))) {
        return 0;
    }
    return this->data[this->pos++];
}

uint8_t BinaryReader::readUInt8() {
    if (!this->checkSize(sizeof(uint8_t))) {
        return 0;
    }
    return (uint8_t) (this->readInt8() & 0xFF);
}

int16_t BinaryReader::readInt16() {
    if (!this->checkSize(sizeof(int16_t))) {
        return 0;
    }
    return (int16_t) (this->readInt8() | this->readInt8() << 8);
}

uint16_t BinaryReader::readUInt16() {
    if (!this->checkSize(sizeof(uint16_t))) {
        return 0;
    }
    return (uint16_t) readInt16();
}

int32_t BinaryReader::readInt32() {
    if (!this->checkSize(sizeof(int32_t))) {
        return 0;
    }
    return (int32_t) (this->readInt8() | this->readInt8() << 8 | this->readInt8() << 16 | this->readInt8() << 24);
}

uint32_t BinaryReader::readUInt32() {
    if (!this->checkSize(sizeof(uint32_t))) {
        return 0;
    }
    return (uint32_t) readInt32();
}

int64_t BinaryReader::readInt64() {
    if (!this->checkSize(sizeof(int64_t))) {
        return 0;
    }
    return (int64_t) (this->readInt8() | this->readInt8() << 8 | this->readInt8() << 16 | this->readInt8() << 24 | this->readInt8() << 32 | this->readInt8() << 40 | this->readInt8() << 48 | this->readInt8() << 56);
}

uint64_t BinaryReader::readUInt64() {
    if (!this->checkSize(sizeof(uint64_t))) {
        return 0;
    }
    return (uint64_t) readInt64();
}

std::string BinaryReader::readString() {
    if (!this->checkSize(sizeof(uint16_t))) {
        return "";
    }
    uint16_t length = this->readUInt16();
    if (!this->checkSize(length)) {
        return "";
    }
    std::string value;
    value.resize(length);
    memcpy(&value[0], &this->data[this->pos], length);
    this->pos += length;
    return value;
}

size_t BinaryReader::read(uint8_t *buffer, size_t length) {
    if (!this->checkSize(length)) {
        return 0;
    }
    memcpy(buffer, &this->data[this->pos], length);
    this->pos += length;
    return length;
}