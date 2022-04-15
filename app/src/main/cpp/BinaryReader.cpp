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

uint8_t BinaryReader::readByte() {
    if (this->pos >= this->size) {
        return 0;
    }
    return this->data[this->pos++];
}

int16_t BinaryReader::readShort() {
    int16_t value = 0;
    value |= this->readByte();
    value |= this->readByte() << 8;
    return value;
}

uint16_t BinaryReader::readUShort() {
    uint16_t value = 0;
    value |= this->readByte();
    value |= this->readByte() << 8;
    return value;
}

int32_t BinaryReader::readInt() {
    int32_t value = 0;
    value |= this->readByte();
    value |= this->readByte() << 8;
    value |= this->readByte() << 16;
    value |= this->readByte() << 24;
    return value;
}

uint32_t BinaryReader::readUInt() {
    uint32_t value = 0;
    value |= this->readByte();
    value |= this->readByte() << 8;
    value |= this->readByte() << 16;
    value |= this->readByte() << 24;
    return value;
}

int64_t BinaryReader::readLong() {
    int64_t value = 0;
    value |= this->readByte();
    value |= this->readByte() << 8;
    value |= this->readByte() << 16;
    value |= this->readByte() << 24;
    value |= this->readByte() << 32;
    value |= this->readByte() << 40;
    value |= this->readByte() << 48;
    value |= this->readByte() << 56;
    return value;
}

uint64_t BinaryReader::readULong() {
    uint64_t value = 0;
    value |= this->readByte();
    value |= this->readByte() << 8;
    value |= this->readByte() << 16;
    value |= this->readByte() << 24;
    value |= this->readByte() << 32;
    value |= this->readByte() << 40;
    value |= this->readByte() << 48;
    value |= this->readByte() << 56;
    return value;
}

std::string BinaryReader::readString() {
    if (this->pos + 2 >= this->size) {
        return "";
    }
    uint16_t length = this->readUShort();
    if (this->pos + length >= this->size) {
        return "";
    }
    std::string value;
    value.resize(length);
    memcpy(&value[0], &this->data[this->pos], length);
    this->pos += length;
    return value;
}

std::vector<uint8_t> BinaryReader::readBytes(size_t n) {
    std::vector<uint8_t> value;
    value.resize(n);
    memcpy(&value[0], &this->data[this->pos], n);
    this->pos += n;
    return value;
}