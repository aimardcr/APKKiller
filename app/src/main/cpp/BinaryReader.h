#include <iostream>
#include <vector>

#ifndef APKKILLER_BINARYREADER_H
#define APKKILLER_BINARYREADER_H

class BinaryReader {
private:
    uint8_t *data;
    size_t size;
    size_t pos;
public:
    BinaryReader(uint8_t *data, size_t size);
    ~BinaryReader();
    int8_t readInt8();
    uint8_t readUInt8();
    int16_t readInt16();
    uint16_t readUInt16();
    int32_t readInt32();
    uint32_t readUInt32();
    int64_t readInt64();
    uint64_t readUInt64();
    std::string readString();
    size_t read(uint8_t *buffer, size_t length);
private:
    bool checkSize(size_t size);
};

#endif //APKKILLER_BINARYREADER_H
