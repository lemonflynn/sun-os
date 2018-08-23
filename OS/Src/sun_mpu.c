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
#include "sun_mpu.h"
#include "sun_common.h"
#include "uart.h"

#define USER_STACK_REGION	2
#define CURR_STACK_REGION	3

int mpu_setup(void)
{
	uint32_t i;
	uint32_t const mpu_cfg_rbar[5] = {
		0x08000000, // Flash address for STM32F4
		0x20000000, // SRAM
		0x20000000, // SRAM
		0x20000000, // SRAM
		0x40000000, // Peripherals 
		//GPIOD_BASE, // GPIO D base address
		//RCC_BASE // Reset Clock CTRL base address
	};

	uint32_t const mpu_cfg_rasr[5] = {
		(MPU_DEFS_RASR_SIZE_1MB | MPU_DEFS_NORMAL_MEMORY_WT |
		MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk), // Flash
		(MPU_DEFS_RASR_SIZE_128KB | MPU_DEFS_NORMAL_MEMORY_WT |
		MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk), // SRAM
		(MPU_DEFS_RASR_SIZE_128KB | MPU_DEFS_NORMAL_MEMORY_WT |
		MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk), // SRAM
		(MPU_DEFS_RASR_SIZE_128KB | MPU_DEFS_NORMAL_MEMORY_WT |
		MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk), // SRAM
		(MPU_DEFS_RASR_SIZE_512MB | MPU_DEFS_SHARED_DEVICE|
		MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk), // Peripherals 
		//(MPU_DEFS_RASR_SIZE_1KB | MPU_DEFS_SHARED_DEVICE |
		//MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk), // GPIO D
		//(MPU_DEFS_RASR_SIZE_1KB | MPU_DEFS_SHARED_DEVICE |
		//MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk) // RCC
	};

	/*Enable MemManage_fault*/
	SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk; // Set bit 16

	if (MPU->TYPE==0) {return PARA_ERR;} // Return 1 to indicate error

	__DMB(); // Make sure outstanding transfers are done

	MPU->CTRL = 0; // Disable the MPU

	for (i=0;i<5;i++) { // Configure only 4 regions
		MPU->RNR = i; // Select which MPU region to configure
		MPU->RBAR = mpu_cfg_rbar[i]; // Configure region base address register
		MPU->RASR = mpu_cfg_rasr[i]; // Configure region attribute and size register
	}

	for (i=5;i<8;i++) {// Disabled unused regions
		MPU->RNR = i; // Select which MPU region to configure
		MPU->RBAR = 0; // Configure region base address register
		MPU->RASR = 0; // Configure region attribute and size register
	}

	MPU->CTRL = MPU_CTRL_ENABLE_Msk; // Enable the MPU

	__DSB(); // Memory barriers to ensure subsequence data & instruction
	__ISB(); // transfers using updated MPU settings

	return 0; // No error
}

int mpu_task_init(uint32_t stack, uint32_t stack_size)
{
	static uint32_t stack_top, stack_bottom;
	uint32_t attr = MPU_DEFS_NORMAL_MEMORY_WT | MPU_DEFS_RASE_AP_NO_ACCESS | MPU_RASR_ENABLE_Msk;
	uint32_t region_size = 0;

	if(stack==0 || stack_size==0)
		return PARA_ERR;
	
	if(stack_bottom==0)
		stack_bottom = stack;

	stack_top = stack+stack_size;

	if(stack_top-stack_bottom<=256)
			region_size = MPU_DEFS_RASR_SIZE_256B;
	else if(stack_top-stack_bottom<=512)
			region_size = MPU_DEFS_RASR_SIZE_512B;
	else if(stack_top-stack_bottom<=1024)
			region_size = MPU_DEFS_RASR_SIZE_1KB;
	else if(stack_top-stack_bottom<=2048)
			region_size = MPU_DEFS_RASR_SIZE_2KB;
	else
		return PARA_ERR;

	/* only supported in gcc
	switch(stack_top-stack_bottom){
		case (0)...(256):
			region_size = MPU_DEFS_RASR_SIZE_256B;
			break;
		case (256)...(512):
			break;
		case (512)...(1024):
			region_size = MPU_DEFS_RASR_SIZE_1KB;
			break;
		case (1024)...(2048):
			region_size = MPU_DEFS_RASR_SIZE_2KB;
			break;
		default:
			return PARA_ERR;
	}
	*/

	MPU->CTRL = 0; // Disable the MPU

	MPU->RNR = USER_STACK_REGION; // Select which MPU region to configure
	MPU->RBAR = stack_bottom; // Configure region base address register
	MPU->RASR = attr|region_size; // Configure region attribute and size register
	MPU->CTRL = MPU_CTRL_ENABLE_Msk; // Enable the MPU

	__DSB(); // Memory barriers to ensure subsequence data & instruction
	__ISB(); // transfers using updated MPU settings

	return 0;
}

int mpu_test_foo(uint32_t addr, uint32_t size)
{
	uint32_t attr = MPU_DEFS_RASR_SIZE_256B | MPU_DEFS_NORMAL_MEMORY_WT | MPU_DEFS_RASE_AP_NO_ACCESS | MPU_RASR_ENABLE_Msk;
	
	MPU->CTRL = 0; // Disable the MPU

	MPU->RNR = USER_STACK_REGION; // Select which MPU region to configure
	MPU->RBAR = addr; // Configure region base address register
	MPU->RASR = attr; // Configure region attribute and size register
	MPU->CTRL = MPU_CTRL_ENABLE_Msk; // Enable the MPU

	__DSB(); // Memory barriers to ensure subsequence data & instruction
	__ISB(); // transfers using updated MPU settings

	return 0;
}

int mpu_task_schedule(uint32_t stack, uint32_t stack_size)
{
	uint32_t attr = MPU_DEFS_NORMAL_MEMORY_WT |
		MPU_DEFS_RASE_AP_FULL_ACCESS | MPU_RASR_ENABLE_Msk;
	uint32_t region_size = 0;

	if(stack==0 || stack_size==0)
		return PARA_ERR;

	if(stack_size<=256)
			region_size = MPU_DEFS_RASR_SIZE_256B;
	else if(stack_size<=512)
			region_size = MPU_DEFS_RASR_SIZE_512B;
	else if(stack_size<=1024)
			region_size = MPU_DEFS_RASR_SIZE_1KB;
	else if(stack_size<=2048)
			region_size = MPU_DEFS_RASR_SIZE_2KB;
	else
		return PARA_ERR;

	MPU->CTRL = 0; // Disable the MPU

	MPU->RNR = CURR_STACK_REGION; // Select which MPU region to configure
	MPU->RBAR = stack; // Configure region base address register
	MPU->RASR = attr|region_size; // Configure region attribute and size register

	MPU->CTRL = MPU_CTRL_ENABLE_Msk; // Enable the MPU

	return 0;
}
