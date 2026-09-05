/*
* Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "SCCircularBuffer.h"

#ifdef PICO_NEEDS_PROTECTOR

#endif

using namespace tccollection;

SCCircularBuffer::SCCircularBuffer(uint16_t size) : readerPosition(0), writerPosition(0), bufferSize(size), buffer(new uint8_t[size]) {
}

SCCircularBuffer::~SCCircularBuffer() {
    delete[] buffer;
}

void SCCircularBuffer::put(uint8_t by) {
    buffer[nextPosition(&writerPosition)] = by;
}

uint8_t SCCircularBuffer::get() {
    return buffer[nextPosition(&readerPosition)];
}

uint16_t SCCircularBuffer::nextPosition(position_ptr_t positionPtr) const {
    bool successfullyUpdated = false;
    uint32_t existing = 0;
    while(!successfullyUpdated) {
        existing = atomicRead32(positionPtr);
        uint32_t newPos = existing + 1;
        if (newPos >= bufferSize) {
            newPos = 0;
        }
        successfullyUpdated = atomicSwap32(positionPtr, existing, newPos);
    }
    return existing;
}
