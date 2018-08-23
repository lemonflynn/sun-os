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
#ifndef __SUN_TASK_H
#define __SUN_TASK_H

#include "stm32f4xx_nucleo_144.h"
#include "sun_common.h"

#define SLICE_TIME      10  /* 10 * 1ms*/

enum sun_state{
    RUNNING,
    READY,
    PENDING,
};

struct sun_tcb{
    uint32_t            stack_base;
    uint32_t            stack_size;
    uint32_t            stack_bottom;
    uint32_t            time_left;
    struct sun_tcb *    next_sun_tcb;
    enum sun_state      state;
    char *              task_name;
};

int32_t create_task(char * name, unsigned int stack, uint32_t stack_size, uint32_t task);
int32_t start_task(unsigned int task_num);
int32_t suspend_task(unsigned int task_num);
int32_t resume_task(unsigned int task_num);
enum sun_state get_task_state(unsigned int task_num);

#endif
