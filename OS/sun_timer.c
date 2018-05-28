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
#include "sun_timer.h"
#include "sun_common.h"

#define MAX_TIMER   32

static int32_t sun_timer_scheduler(uint32_t ms, struct sun_timer * timer_in);

static struct sun_timer timer_pool[MAX_TIMER];
static struct sun_timer * free_timer_list;
static struct sun_timer * event_queue;

void sun_timer_init(void)
{
    int i;
    for(i=0;i<MAX_TIMER-1;i++){
        timer_pool[i].next = &timer_pool[i+1];
    }
    timer_pool[MAX_TIMER-1].next = NULL;

    free_timer_list = &timer_pool[0];
}

void sun_timer_handler(void)
{
    struct sun_timer * next_timer, * expired_timer;
    if(event_queue == NULL)
        return;

    if(event_queue->delta > 0)
        event_queue->delta--;

    next_timer = event_queue;
    while(next_timer && !next_timer->delta){
        next_timer->handler(next_timer->data);
        expired_timer = next_timer;
        next_timer = next_timer->next;
        event_queue = next_timer;
        sun_timer_free(expired_timer);
    }
}

int32_t sun_timer_scheduler(uint32_t ms, struct sun_timer * timer_in)
{
    struct sun_timer * timer, * next_timer = event_queue;
    uint32_t raw_time = 0, delta = 0;

    if(ms == 0)
        return PARA_ERR;

    if(timer_in == NULL)
        return PARA_ERR;

    if(event_queue == NULL){
        timer_in->delta = ms;
        event_queue = timer_in;
        return NO_ERR;
    }

    if(ms < next_timer->delta){
        timer_in->next = event_queue;
        timer_in->delta = ms;
        event_queue->delta -= ms;
        event_queue = timer_in;

        return NO_ERR;
    }

loop:
    timer = next_timer;
    raw_time += timer->delta;
    delta = ms - raw_time;
    next_timer = timer->next;
    if(next_timer != NULL && ms > raw_time + next_timer->delta)
        goto loop;

    timer_in->delta = delta;
    timer->next = timer_in;
    timer_in->next = next_timer;
    next_timer->delta -= delta;
    return NO_ERR;
}

struct sun_timer * sun_timer_malloc(uint32_t ms, sun_timer_handler_t handler, void * data)
{
    struct sun_timer * timer;
    if(ms == 0)
        return NULL;

    if(free_timer_list == NULL)
        return NULL;

    timer = free_timer_list;
    free_timer_list = free_timer_list->next;

    timer->handler = handler;
    timer->data = data;
    timer->next = NULL;

    sun_timer_scheduler(ms, timer);

    return timer;
}

int32_t sun_timer_free(struct sun_timer * timer)
{
    if(timer == NULL)
        return PARA_ERR;

    timer->next = free_timer_list;
    free_timer_list = timer;

    return NO_ERR;
}
