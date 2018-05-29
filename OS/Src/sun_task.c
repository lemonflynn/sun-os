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
#include "sun_task.h"

#define MAX_TASK_NUM  10
#define HW32_REG(ADDRESS) (*((volatile unsigned long *)(ADDRESS)))

struct sun_tcb tcb_list[MAX_TASK_NUM];
struct sun_tcb * curr_tcb;
struct sun_tcb * next_tcb;
uint8_t sun_os_start = FALSE;

static uint32_t task_cnt;

int32_t create_task(unsigned int stack, uint32_t stack_size, uint32_t task)
{
    if(stack == 0 || stack_size == 0 || task == 0)
        return PARA_ERR;

    tcb_list[task_cnt].stack_base = stack + stack_size - 16*4;
    tcb_list[task_cnt].state = RUNNING;
    /* initial pc */
    HW32_REG((tcb_list[task_cnt].stack_base + (14<<2))) = task;
    /* initial xPSR */
    HW32_REG((tcb_list[task_cnt].stack_base + (15<<2))) = 0x01000000;

    tcb_list[0].next_sun_tcb = &tcb_list[task_cnt];

    if(0 != task_cnt)
        tcb_list[task_cnt].next_sun_tcb = &tcb_list[task_cnt-1];

    task_cnt++;

    return NO_ERR;
}

int32_t start_task(unsigned int task_num)
{
    if(task_num > MAX_TASK_NUM - 1)
        return PARA_ERR;

    curr_tcb = &tcb_list[task_num];

    __set_PSP((curr_tcb->stack_base + 16*4));

    __set_CONTROL(0x3);

    __ISB();

    sun_os_start = TRUE;

    return NO_ERR;
}

int32_t suspend_task(unsigned int task_num)
{
    if(task_num > MAX_TASK_NUM - 1)
        return PARA_ERR;

    tcb_list[task_num].state = PENDING;
    return NO_ERR;
}

int32_t resume_task(unsigned int task_num)
{
    if(task_num > MAX_TASK_NUM - 1)
        return PARA_ERR;

    tcb_list[task_num].state = RUNNING;
    return NO_ERR;
}

enum sun_state get_task_state(unsigned int task_num)
{
    return tcb_list[task_num].state;
}
