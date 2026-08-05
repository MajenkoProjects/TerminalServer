#include <string.h>

#include "app.h"
#include "port.h"
#include "leds.h"

struct port *ports = NULL;

#define MUTEX_TICKS 100

int cb_available(struct circular_buffer *buf) {
    int o = 0;
    if (xSemaphoreTake(buf->mutex, MUTEX_TICKS) == pdTRUE) {
        o = (PORT_BUFFER_SIZE + buf->head - buf->tail) % PORT_BUFFER_SIZE;
        xSemaphoreGive(buf->mutex);
    }
    led_num(o);
    return o;
}

int cb_free(struct circular_buffer *buf) {
    int o = 0;
    if (xSemaphoreTake(buf->mutex, MUTEX_TICKS) == pdTRUE) {
        o = (PORT_BUFFER_SIZE + buf->tail - buf->head) % PORT_BUFFER_SIZE;
        xSemaphoreGive(buf->mutex);
    }
    return o;
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
    struct port *newport = malloc(sizeof(struct port));
    memset(newport, 0, sizeof(struct port));
    
    newport->type = type;
    newport->port_data = data;
    
    newport->read_buffer.mutex = xSemaphoreCreateMutex();
    newport->write_buffer.mutex = xSemaphoreCreateMutex();
    
    if (ports == NULL) {
        ports = newport;
        return newport;
    }
    
    struct port *scan = ports;
    while (scan->next) scan = scan->next;
    scan->next = newport;
    return newport;
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