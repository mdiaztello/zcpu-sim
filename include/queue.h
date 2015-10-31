
#ifndef __QUEUE_H_
#define __QUEUE_H_

// An implementation of the circular queue data structure for general use

#include <stdbool.h>
#include <stdint.h>

typedef struct queue_t queue_t;

enum return_data_status_t { QUEUE_IS_EMPTY_ERROR = -1, QUEUE_DATA_RETRIEVED_SUCCESSFULLY };

struct queue_return_data_t
{
    enum return_data_status_t return_data_status;
    uint8_t value;
};

typedef struct queue_return_data_t queue_return_data_t;

queue_t* queue_create(uint8_t queue_size);
void queue_destroy(queue_t* queue);

bool queue_is_empty(queue_t* queue);
bool queue_is_full(queue_t* queue);

uint8_t queue_get_size(queue_t* queue);

bool queue_put(queue_t* queue, uint8_t value);
queue_return_data_t queue_get(queue_t* queue);

#endif // __QUEUE_H_
