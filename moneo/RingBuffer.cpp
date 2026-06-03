#include "RingBuffer.h"
#include "Config.h"

RingBuffer::RingBuffer(size_t capacity)
    : _buf(nullptr), _capacity(capacity), _head(0), _tail(0), _mutex(nullptr) {}

RingBuffer::~RingBuffer() {
    if (_buf) free(_buf);
    if (_mutex) vSemaphoreDelete(_mutex);
}

bool RingBuffer::begin() {
    // Allocate from PSRAM if available, otherwise heap
    if (psramFound()) {
        _buf = (uint8_t*)ps_malloc(_capacity);
        DLOG("[RingBuffer] Allocated in PSRAM.");
    } else {
        _buf = (uint8_t*)malloc(_capacity);
        DLOG("[RingBuffer] WARNING: PSRAM not found, using heap.");
    }

    if (!_buf) {
        DLOG("[RingBuffer] FATAL: Buffer allocation failed!");
        return false;
    }

    _mutex = xSemaphoreCreateMutex();
    if (!_mutex) {
        DLOG("[RingBuffer] FATAL: Mutex creation failed!");
        return false;
    }

    _head = 0;
    _tail = 0;
    return true;
}

size_t RingBuffer::write(const uint8_t* data, size_t len) {
    if (!_buf || !len) return 0;

    xSemaphoreTake(_mutex, portMAX_DELAY);

    size_t written = 0;
    for (size_t i = 0; i < len; i++) {
        size_t next = (_head + 1) % _capacity;
        if (next == _tail) break;   // buffer full, drop remaining
        _buf[_head] = data[i];
        _head = next;
        written++;
    }

    xSemaphoreGive(_mutex);
    return written;
}

size_t RingBuffer::read(uint8_t* out, size_t len) {
    if (!_buf || !len) return 0;

    xSemaphoreTake(_mutex, portMAX_DELAY);

    size_t bytesRead = 0;
    while (bytesRead < len && _tail != _head) {
        out[bytesRead++] = _buf[_tail];
        _tail = (_tail + 1) % _capacity;
    }

    xSemaphoreGive(_mutex);
    return bytesRead;
}

size_t RingBuffer::available() const {
    if (_head >= _tail) return _head - _tail;
    return _capacity - _tail + _head;
}

size_t RingBuffer::freeSpace() const {
    return _capacity - available() - 1;
}

bool RingBuffer::isEmpty() const {
    return _head == _tail;
}

void RingBuffer::reset() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _head = 0;
    _tail = 0;
    xSemaphoreGive(_mutex);
}
