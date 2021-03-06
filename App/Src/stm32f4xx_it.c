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
#include "main.h"
#include "stm32f4xx_it.h"
#include "sun_task.h"
#include "sun_timer.h"
#include "sun_mpu.h"

extern struct sun_tcb * curr_tcb;
extern struct sun_tcb * next_tcb;
extern volatile uint32_t systick_count;
extern uint8_t sun_os_start;

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
 	while (1)
		__asm("BKPT #0\n");
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
 	while (1)
		__asm("BKPT #0\n");
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
__asm void PendSV_Handler(void)
{
		import mpu_task_schedule
    //Save current context
    MRS     R0, PSP         //get current process stack point value
    STMDB   R0!, {R4-R11}   //save r4-r11 in task stack
    LDR     R1, =__cpp(&curr_tcb) //get the address of curr_tcb
    LDR     R7,[R1]         //get the value of curr_tcb
    STR     R0,[R7]         //save the stack base

    LDR     R4,=__cpp(&next_tcb)
    LDR     R4,[R4]
    STR     R4,[R1]         //set curr_task = next_task

	PUSH 	{R5-R7,lr}
	/*Mpu schedule*/
	LDR		R0,[R4, #8]
	LDR		R1,[R4,	#4]
	BL		mpu_task_schedule
	POP 	{R5-R7,lr}

    //load next context
    LDR     R0,[R4]
    LDMIA   R0!,{R4-R11}

    MSR     PSP, R0
    BX      LR
    ALIGN   4
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    HAL_IncTick();
    sun_timer_handler();

    systick_count++;
    if(!sun_os_start)
        return;

	/*decrease current time remained*/
    if(curr_tcb->time_left--)
    	return;

	/*current task has run out of time, time to schedule..*/
	curr_tcb->time_left = SLICE_TIME;

    for(next_tcb = curr_tcb->next_sun_tcb; next_tcb->state == PENDING; next_tcb = next_tcb->next_sun_tcb);

    if(curr_tcb != next_tcb)
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(USER_BUTTON_PIN);
}
