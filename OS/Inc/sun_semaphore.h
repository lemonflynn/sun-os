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
#ifndef __SUN_SEM_H
#define __SUN_SEM_H

#include "sun_task.h"

struct semaphore{
    uint32_t    cnt;
    struct sun_tcb *   waiting_list;
};

void init_semaphore(struct semaphore * sem, uint32_t count);
int32_t pend_semaphore(struct semaphore * sem, uint32_t timeout);
int32_t post_semaphore(struct semaphore * sem);

#endif
