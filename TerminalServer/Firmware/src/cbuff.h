#ifndef _CBUFF_H
#define _CBUFF_H

#include <stdint.h>
#include <FreeRTOS.h>
#include <semphr.h>


#define CIRCULAR_BUFFER_SIZE 512
#define MUTEX_TICKS 100


struct circular_buffer {
    uint8_t data[CIRCULAR_BUFFER_SIZE];
    int head;
    int tail;
    SemaphoreHandle_t mutex;
};

extern int cb_write(struct circular_buffer *buf, uint8_t b);
extern int cb_read(struct circular_buffer *buf);
extern int cb_free(struct circular_buffer *buf);
extern int cb_available(struct circular_buffer *buf);
extern int cb_available(struct circular_buffer *buf);

#endif