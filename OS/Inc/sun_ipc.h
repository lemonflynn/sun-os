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
#ifndef __SUN_IPC_H
#define __SUN_IPC_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo_144.h"

struct message{
    struct message * next;
    void * data;
    uint32_t size;
};

struct msg_queue{
    struct message * head;
    struct message * tail;
    uint32_t usage;
    uint32_t size;
};

void msg_pool_init(void);
int32_t init_msg_queue(struct msg_queue * queue, uint32_t size);
int32_t push_msg_queue(struct msg_queue * queue, void * data, uint32_t size);
int32_t pop_msg_queue(struct msg_queue * queue, void ** data, uint32_t * size);

#endif
