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
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo_144.h"

enum sun_state{
    RUNNING,
    READY,
    PENDING,
};

struct sun_tcb{
    uint32_t            stack_base;
    uint32_t            stack_size;
    struct sun_tcb *    next_sun_tcb;
    enum sun_state      state;
};
void create_task(unsigned int stack, uint32_t stack_size, uint32_t task);
void start_task(unsigned int task_num);
