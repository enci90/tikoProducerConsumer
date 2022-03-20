#ifndef CIRC_BUFF_INC_H // avoid multiple inclusion

#define CIRC_BUFF_INC_H
// define BUFFER_SIZE for sake of clarity and for sake of safety.

// Circular buffer structure
typedef struct circular_buffer_t circular_buffer_t;

// Handle type, the way users interact with the API
typedef circular_buffer_t* cbuffer_handle_t;

// In: buffer and its size
// Ret: a circular buffer handler 
cbuffer_handle_t circular_buffer_init(int* buffer, size_t size);

// to be called when cbuffer is not used anymore
// actual buffer need to be de allocated by application 
void circular_buffer_free(cbuffer_handle_t buff);

// Put try to store data in buff.
// Ret: 0 ok, -1 if buffer is full
int circular_buffer_try_put(cbuffer_handle_t buff, int data);

// Get a value from the buffer
// Ret: 0 ok, -1 if the buffer is empty
int circular_buffer_get(cbuffer_handle_t buff, int* data);

#endif 