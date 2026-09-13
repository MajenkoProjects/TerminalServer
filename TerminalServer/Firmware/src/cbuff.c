#include "cbuff.h"


int cb_available(struct circular_buffer *buf) {
    int o = 0;
    if (xSemaphoreTake(buf->mutex, MUTEX_TICKS) == pdTRUE) {
        o = (CIRCULAR_BUFFER_SIZE + buf->head - buf->tail) % CIRCULAR_BUFFER_SIZE;
        xSemaphoreGive(buf->mutex);
    }
    return o;
}

int cb_free(struct circular_buffer *buf) {
    if (xSemaphoreTake(buf->mutex, MUTEX_TICKS) == pdTRUE) {
        size_t newhead = (buf->head + 1) % CIRCULAR_BUFFER_SIZE;
        if (newhead != buf->tail) {
            xSemaphoreGive(buf->mutex);
            return 1;
        } else {
            xSemaphoreGive(buf->mutex);
            return 0;
        }
    }
    return 0;
}

// Add a new byte to the circular buffer. Returns 1 for a byte
// written or -1 for no room for the byte
int cb_write(struct circular_buffer *buf, uint8_t b) {
    if (xSemaphoreTake(buf->mutex, MUTEX_TICKS) == pdTRUE) {
        size_t newhead = (buf->head + 1) % CIRCULAR_BUFFER_SIZE;
        if (newhead != buf->tail) {
            buf->data[buf->head] = b;
            buf->head = newhead;

            xSemaphoreGive(buf->mutex);
            return 1;
        } else {
            xSemaphoreGive(buf->mutex);
            return -1;
        }
    }
    return -1;
}

// Read a byte from the circular buffer. Returns -1 if no
// byte available to read
int cb_read(struct circular_buffer *buf) {
    if (xSemaphoreTake(buf->mutex, MUTEX_TICKS) == pdTRUE) {
        if (buf->head == buf->tail) {
            xSemaphoreGive(buf->mutex);
            return -1;
        }
        
        int d = buf->data[buf->tail];
        buf->tail = (buf->tail + 1) % CIRCULAR_BUFFER_SIZE;
        xSemaphoreGive(buf->mutex);
        return d;
    }
    return -1;
}
