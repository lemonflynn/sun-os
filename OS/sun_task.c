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

static uint32_t task_cnt;

void create_task(unsigned int stack, uint32_t stack_size, uint32_t task)
{
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
}

void start_task(unsigned int task_num)
{
    curr_tcb = &tcb_list[task_num];
  __set_PSP((curr_tcb->stack_base + 16*4));

  __set_CONTROL(0x3);

  __ISB();
}
