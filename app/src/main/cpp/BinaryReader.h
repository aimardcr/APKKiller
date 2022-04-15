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
    uint8_t readByte();
    int16_t readShort();
    uint16_t readUShort();
    int32_t readInt();
    uint32_t readUInt();
    int64_t readLong();
    uint64_t readULong();
    std::string readString();
    std::vector<uint8_t> readBytes(size_t n);
};

#endif //APKKILLER_BINARYREADER_H
