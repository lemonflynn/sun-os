/*
 * sun operating system
 *
 * Copyright (c) Siemens AG, 2018, 2019
 *
 * Authors:
 *  Francisco flynn<lin.xu@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */
#include "sun_ipc.h"
#include "sun_common.h"

#define MAX_MESSAGE 100

struct message msg_pool[MAX_MESSAGE];
struct message * free_msg_list;

void msg_pool_init(void)
{
    int i;
    for(i=0;i<MAX_MESSAGE-1;i++){
        msg_pool[i].next = &msg_pool[i+1];
    }
    msg_pool[MAX_MESSAGE-1].next = NULL;

    free_msg_list = &msg_pool[0];
}

int32_t init_msg_queue(struct msg_queue * queue, uint32_t size)
{
    if(queue == NULL)
        return PARA_ERR;

    if(size == 0)
        return PARA_ERR;

    queue->head = NULL;
    queue->tail = NULL;
    queue->usage = 0;
    queue->size = size;

    return NO_ERR;
}

int32_t push_msg_queue(struct msg_queue * queue, void * data, uint32_t size)
{
    struct message * msg = free_msg_list;

    if(queue == NULL || data == NULL)
        return PARA_ERR;

    if(queue->usage >= queue->size)
        return QUEUE_FULL_ERR;

    if(msg == NULL)
        return MSG_POOL_EMPTY;

    free_msg_list = free_msg_list->next;

    msg->next = NULL;
    msg->data = data;
    msg->size = size;

    /*queue is empty now*/
    if(0 == queue->usage){
        queue->head = queue->tail = msg;
        goto ret;
    }

    /* insert into queue list */
    queue->tail->next = msg;
    /* update tail cursor */
    queue->tail = msg;

ret:
    queue->usage++;

    return NO_ERR;
}

int32_t pop_msg_queue(struct msg_queue * queue, void ** data, uint32_t * size)
{
    struct message * msg = NULL;

    if(queue == NULL || size == NULL)
        return PARA_ERR;

    if(queue->usage == 0)
        return QUEUE_EMPTY_ERR;

    msg = queue->head;
    *data = msg->data;
    *size = msg->size;

    queue->head = msg->next;
    queue->usage--;

    msg->next = free_msg_list;
    msg->data = NULL;
    msg->size = 0;
    free_msg_list = msg;

    return NO_ERR;
}
