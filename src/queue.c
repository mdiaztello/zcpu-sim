
// ----------------------------------------------------------------------------
//
//  FILE: queue.c
//
//  DESCRIPTION: This module is a general purpose implementation of a queue
//  data structure, but I intend to use it to implement my interrupt controller
//  module. I decided to put this code in a separate module so that it could be
//  tested separately from the interrupt controller logic. I also separated
//  this module in case a queue may be useful in other parts of the code.
//
// ----------------------------------------------------------------------------


#include "queue.h"
#include <stdlib.h>


struct queue_t
{
    size_t size;
    size_t num_elements_in_queue;
    uint8_t* data;
    size_t head;
    size_t tail;
};

queue_t* queue_create(size_t queue_size)
{
    if(queue_size < 1)
        return NULL;

    queue_t* queue = calloc(1, sizeof(queue_t));
    queue->data = calloc(1, queue_size*sizeof(*(queue->data)));
    queue->size = queue_size;
    queue->num_elements_in_queue = 0;
    queue->head = 0;
    queue->tail = 0;
    return queue;
}

void queue_destroy(queue_t* queue)
{
    free(queue->data);
    free(queue);
}

bool queue_is_empty(queue_t* queue)
{
    return queue->num_elements_in_queue == 0;
}

bool queue_is_full(queue_t* queue)
{
    return queue->num_elements_in_queue == queue->size;
}

uint8_t queue_get_size(queue_t* queue)
{
    return queue->size;
}

bool queue_put(queue_t* queue, uint8_t value)
{
    bool result = false;
    if(!queue_is_full(queue))
    {
        queue->data[queue->tail] = value;
        queue->tail = (queue->tail + 1) % queue->size;
        queue->num_elements_in_queue++;
        result = true;
    }
    return result;
}

queue_return_data_t queue_get(queue_t* queue)
{
    queue_return_data_t return_data;
    return_data.return_data_status = QUEUE_IS_EMPTY_ERROR;

    if(!queue_is_empty(queue))
    {
        queue->num_elements_in_queue--;
        return_data.return_data_status = QUEUE_DATA_RETRIEVED_SUCCESSFULLY;
        return_data.value = queue->data[queue->head];
        queue->head = (queue->head + 1) % queue->size;
    }

    return return_data;
}
