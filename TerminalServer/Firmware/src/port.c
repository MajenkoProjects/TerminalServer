#include <string.h>

#include "app.h"
#include "port.h"
#include "leds.h"
#include "pin.h"

struct port ports[MAX_PORTS] = {0};

#define MUTEX_TICKS 100

int cb_available(struct circular_buffer *buf) {
    int o = 0;
    if (xSemaphoreTake(buf->mutex, MUTEX_TICKS) == pdTRUE) {
        o = (PORT_BUFFER_SIZE + buf->head - buf->tail) % PORT_BUFFER_SIZE;
        xSemaphoreGive(buf->mutex);
    }
    return o;
}

int cb_free(struct circular_buffer *buf) {
    if (xSemaphoreTake(buf->mutex, MUTEX_TICKS) == pdTRUE) {
        size_t newhead = (buf->head + 1) % PORT_BUFFER_SIZE;
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
        size_t newhead = (buf->head + 1) % PORT_BUFFER_SIZE;
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
        buf->tail = (buf->tail + 1) % PORT_BUFFER_SIZE;
        xSemaphoreGive(buf->mutex);
        return d;
    }
    return -1;
}

// Write a block of data to the port. Returns the actual number
// of bytes written to the port.
int port_write(struct port *port, uint8_t *data, size_t len) {
    size_t buf_free = cb_free(&(port->write_buffer));
    if (len > buf_free) {
        len = buf_free;
    }
    
    for (int i = 0; i < len; i++) {
        cb_write(&port->write_buffer, data[i]);
    }
    return len;
}

// Read up-to len bytes from the port. Returns the actual number
// of bytes read
int port_read(struct port *port, uint8_t *data, size_t len) {    
    size_t avail = cb_available(&port->read_buffer);
    if (len > avail) {
        len = avail;
    }
    
    for (int i = 0; i < len; i++) {
        data[i] = cb_read(&port->read_buffer);
    }
    return len;
}

int port_read_byte(struct port *port) {
    return cb_read(&(port->read_buffer));
}

int port_write_byte(struct port *port, uint8_t b) {
    return cb_write(&(port->write_buffer), b);
}

struct port *add_port(enum port_type type, void *data) {

    
    for (int i = 0; i < MAX_PORTS; i++) {
        if (ports[i].type == PORT_NONE) {
            ports[i].type = type;
            ports[i].no = i;
            ports[i].port_data = data;
            ports[i].read_buffer.mutex = xSemaphoreCreateMutex();
            ports[i].write_buffer.mutex = xSemaphoreCreateMutex();
            return &ports[i];
        }
    }
    


    return NULL;
}

int port_available(struct port *port) {
    return cb_available(&port->read_buffer);
}

int port_printf(struct port *port, const char *fmt, ...) {
    va_list ap;
    int count = 0;
    
    // Calculate the amount of space needed
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) return -1;

    // Allocate that plus one byte on the stack
    char *str = alloca(n + 1);
    // Print it
    va_start(ap, fmt);
    vsnprintf(str, n+1, fmt, ap);
    va_end(ap);

    // Send it to the output buffer
    for (int i = 0; i < n; i++) {
        if (cb_write(&port->write_buffer, str[i])) {
            count++;
        }
    }

    return count;
}