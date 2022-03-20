
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "circular_buffer.h"

#define USE_FULL_FLAG

// The definition of our circular buffer structure is hidden from the user
struct circular_buffer_t
{
	int* buffer;
	size_t head;
	size_t tail;
	#ifdef USE_FULL_FLAG
	bool full_flag;
	#endif
	size_t max; // of the buffer
};

// Private functions 
static inline size_t increment_headtail_val(size_t value, size_t max)
{
	if(++value == max)
	{
		value = 0;
	}

	return value;
}
static bool circular_buffer_full(cbuffer_handle_t buff)
{
	
	#ifdef USE_FULL_FLAG
	return buff->full_flag;
	#else
	return increment_headtail_val(buff->head, buff->max) == buff->tail;
	#endif
}

static int circular_buffer_empty(cbuffer_handle_t buff)
{
	assert(buff);
	#ifdef USE_FULL_FLAG
	return (!circular_buffer_full(buff) && (buff->head == buff->tail));
	#else
	return buff->head == buff->tail;
	#endif
}


//

cbuffer_handle_t circular_buffer_init(int* buffer, size_t size)
{
	assert(buffer && size > 1);

	cbuffer_handle_t cbuffer = malloc(sizeof(circular_buffer_t));
	assert(cbuffer);

	cbuffer->buffer = buffer;
	cbuffer->max = size;
	cbuffer->head = 0;
	cbuffer->tail = 0;
	cbuffer->full_flag = false;

	assert(circular_buffer_empty(cbuffer));

	return cbuffer;
}

void circular_buffer_free(cbuffer_handle_t buff)
{
	assert(buff);
	free(buff);
}

int circular_buffer_try_put(cbuffer_handle_t buff, int data)
{
	assert(buff && buff->buffer);

	int r = -1;

	if(!circular_buffer_full(buff))
	{
		buff->buffer[buff->head] = data;
		buff->head = increment_headtail_val(buff->head, buff->max);
		r = 0;
	}

	return r;
}

int circular_buffer_get(cbuffer_handle_t buff, int* data)
{
	assert(buff && data && buff->buffer);

	int r = -1;

	if(!circular_buffer_empty(buff))
	{
		*data = buff->buffer[buff->tail];
		buff->tail = increment_headtail_val(buff->tail, buff->max);
		buff->full_flag = false;
		r = 0;
	}

	return r;
}

