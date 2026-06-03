#ifndef RingBuffer_h
#define RingBuffer_h

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ============================================================
// RingBuffer — PSRAM-backed circular audio buffer
//
// Audio capture task writes here at highest priority.
// Chunk writer task reads from here independently.
// Both sides never block each other for long.
// ============================================================

class RingBuffer {
public:
    RingBuffer(size_t capacity);
    ~RingBuffer();

    bool begin();

    // Write audio samples from ISR/high-priority task.
    // Returns bytes actually written (may be less if nearly full).
    size_t write(const uint8_t* data, size_t len);

    // Read audio samples into output buffer.
    // Returns bytes actually read.
    size_t read(uint8_t* out, size_t len);

    size_t available() const;
    size_t freeSpace() const;
    bool   isEmpty()   const;
    void   reset();

private:
    uint8_t*         _buf;
    size_t           _capacity;
    volatile size_t  _head;   // write index
    volatile size_t  _tail;   // read index
    SemaphoreHandle_t _mutex;
};

#endif
