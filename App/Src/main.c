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
#include "sun_task.h"
#include "sun_semaphore.h"
#include "sun_timer.h"
#include "sun_ipc.h"
#include "sun_mpu.h"
#include "led.h"
#include "uart.h"

#define STOP_CPU __asm("BKPT #0\n")

static void SystemClock_Config(void);
static void Error_Handler(void);
static void EXTI15_10_IRQHandler_Config(void);

static void task0(void);
static void task1(void);
static void task2(void);

int32_t timer1_cb(void * data);
int32_t timer2_cb(void * data);

volatile uint32_t systick_count;
long long task0_stack[32] __attribute__((aligned(4*1024))); 
long long task1_stack[32] __attribute__((aligned(8*32)));  
long long task2_stack[32] __attribute__((aligned(8*32))); 
long long task3_stack[32] __attribute__((aligned(8*32))); 

struct semaphore sem1;
struct msg_queue msg_queue1;

static char banner[] =
	"\n"
	"============================================\r\n"
	" Copyright(C) 2018-2019 The Sun OS Project  \r\n"
	"============================================\r\n"
    "Author: Francisco Flynn \r\n"
	"Build: "  __DATE__ " " __TIME__"\r\n";

char * msg = "flynn";
char message_test[32] = "";
static uint32_t mpu_test;
int foo1(int limit)
{
    int i;
    int ret = 1;

    for(i=0;i<limit;i++)
        ret = ret*i;

    return ret;
}

int foo2(int limit)
{
    int i;
    int ret = 1;

    for(i=limit;i!=0;i--)
        ret = ret*i;

    return ret;
}

int main(void)
{
    volatile int ret;
    /* Enable double word stack alignment, recommended in Cortex-M3 r1p1,
       default in Cortex-M3 r2px and Cortex-M4*/
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;

    /* STM32F4xx HAL library initialization:
         - Configure the Flash prefetch
         - Systick timer is configured by default as source of time base, but user
           can eventually implement his proper time base source (a general purpose
           timer for example or other time source), keeping in mind that Time base
           duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
           handled in milliseconds basis.
         - Set NVIC Group Priority to 4
         - Low Level Initialization
       */
    HAL_Init();

    /* Configure the system clock to 180 MHz */
    SystemClock_Config();

    NVIC_SetPriority(PendSV_IRQn, 0xFF);

    led_init();

    EXTI15_10_IRQHandler_Config();

    uart_init(115200);

    init_semaphore(&sem1, 10);

    sun_timer_init();

    msg_pool_init();

    init_msg_queue(&msg_queue1, 10);

    mpu_setup();

    ret = foo1(100);
    ret = foo2(100);
    /*it's not necessary to init task0's stack frame, because it use the same
    stack frame as main function, put it here is just for consistency*/
    create_task("task0", (unsigned int)task0_stack, sizeof(task0_stack), (uint32_t)task0);
    create_task("task1", (unsigned int)task1_stack, sizeof(task1_stack), (uint32_t)task1);
    create_task("task2", (unsigned int)task2_stack, sizeof(task2_stack), (uint32_t)task2);

    sun_timer_malloc(5000, timer1_cb, msg);
    sun_timer_malloc(1000, timer2_cb, msg);
    start_task(0);

    __set_CONTROL(0x3);
    __ISB();
		
    task0();
    while(1){
        STOP_CPU;
    }
}

int32_t timer1_cb(void * data)
{
    static uint32_t i;
    //printf("%s this is 5s timer cb\n", (char *)data);
    sun_timer_malloc(5000, timer1_cb, msg);

    sprintf(message_test, "this is %d times message\n", i++);
    printf("push message\n\t %s", message_test);
    push_msg_queue(&msg_queue1, message_test, sizeof(message_test));

    return 0;
}

int32_t timer2_cb(void * data)
{
    char * message = NULL;
    uint32_t size;
    int32_t ret = 0;
    //printf("%s this is 1s timer cb\n", (char *)data);
    sun_timer_malloc(1000, timer2_cb, msg);

    ret = pop_msg_queue(&msg_queue1, (void **)&message, &size);
    if(ret != NO_ERR)
        return ret;

    printf("Get a message:\n \t %s", message);

    return 0;
}

void task0(void)
{
    uint32_t old_time = 0;
    /* Output a message on Hyperterminal using printf function */
    printf("%s", banner);
    printf("\r\nstack info:\r\n %x\r\n %x\r\n %x\r\n", &task0_stack[0], &task1_stack[0], &task2_stack[0]);

    while(1){
        if(systick_count>old_time+800){
            //pend_semaphore(&sem1, 100);
            printf("task0 sem cnt %d \n", sem1.cnt);
            HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
            old_time = systick_count;
        }
		if(mpu_test==1)
			task3_stack[31] = 666;
    }
}

void task1(void)
{
    uint32_t old_time = 0;
    while(1){
        if(systick_count>old_time+1600){
            HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
            old_time = systick_count;
        }
    }
}

void task2(void)
{
    uint32_t old_time = 0;
    while(1){
        if(systick_count>old_time+3200){
            HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_PIN);
            old_time = systick_count;
        }
    }
}

static void EXTI15_10_IRQHandler_Config(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;

    /* Enable GPIOC clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure PC.13 pin as input floating */
    GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Enable and set EXTI line 15_10 Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {
        if(get_task_state(2) == RUNNING){
            suspend_task(2);
        }else{
            resume_task(2);
        }
		printf("MPU test\r\n");
		mpu_test = 1;

        post_semaphore(&sem1);
        printf("IRQ sem cnt %d \n", sem1.cnt);
    }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	   clocked below the maximum system frequency, to update the voltage scaling value
	   regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 360;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
	  /* Initialization Error */
	  Error_Handler();
	}

	if(HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
	  /* Initialization Error */
	  Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	   clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
	  /* Initialization Error */
	  Error_Handler();
	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
	/* Turn LED1 off */
 	BSP_LED_Off(LED1);
 	while(1)
 	{
 	}
}
