#ifndef CIRC_BUFF_INC_H // avoid multiple inclusion

#define CIRC_BUFF_INC_H
// define BUFFER_SIZE for sake of clarity and for sake of safety.
#define WHOLE_BUFFER_SIZE       1024
#define CIRC_BUFFER_SIZE		256


typedef struct {
	// to use a circular buffer, I need to track head and tail
	int head;
	int tail;
	int count;
	int buffer[CIRC_BUFFER_SIZE];
} circular_buffer_t;
#endif 