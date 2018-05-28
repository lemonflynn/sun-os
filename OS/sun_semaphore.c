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
#include "sun_semaphore.h"

void init_semaphore(struct semaphore * sem, uint32_t count)
{
    sem->cnt = count;
    sem->waiting_list = NULL;
}

int32_t pend_semaphore(struct semaphore * sem, uint32_t timeout)
{
    while(sem->cnt == 0);

    if(sem->cnt > 0)
        sem->cnt--;

    return 0;
}

int32_t post_semaphore(struct semaphore  * sem)
{
    sem->cnt++;
    return 0;
}
