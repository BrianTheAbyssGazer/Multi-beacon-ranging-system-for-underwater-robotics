/*
 * global_buffer_def.h
 *
 *  Created on: Aug 14, 2024
 *      Author: arthur
 */

#include "mode.h"
#ifndef CONSTANTS_H  // Header Guard: Prevents double inclusion in ONE file
#define CONSTANTS_H

#if  ADC_OVER_UART_MODE
	#define BUF_LEN 24000 //don't forget to change FRAME_LEN in python code too
	extern uint16_t buf[BUF_LEN]; //24k elems, or 48Kbytes

#elif BASIC_PULSE_STREAM_MODE
	#define BUF_LEN 1<<13
	extern uint32_t buf[BUF_LEN]; //about 8k elems, or 32Kbytes

#elif BASIC_PEAK_DETECTOR_MODE
	#define BUF_LEN 18000
	extern uint16_t buf[BUF_LEN]; //18k elems, or 36Kbytes

#elif TIME_OF_FLIGHT_MODE || TRANSPONDER_MODE
	#define BUF_LEN 12288 // 16384 = 2^14
	#define HAL_BUF_LEN 6144 // 16384 = 2^14
	extern int16_t buf[BUF_LEN]; //18k elems, or 36k bytes
	#define OUT_BUF_LEN 2048 // 4096 = 2^12
	#define HAL_OUT_BUF_LEN 1024 // 2048 = 2^11
	#define OUT_BUF_MASK 2047
	extern uint32_t out_buf[OUT_BUF_LEN]; //3k elems, or 12k bytes
	#define STRING_LEN 2

    #define DATA_LEN 8*STRING_LEN
	#define N_CYCLE 8 //cycle per symbol
	#define DEAD_INTERVAL 512 // 2**8 interval between bits
	#define DATA_PFX (DATA_LEN/(OUT_BUF_LEN/DEAD_INTERVAL)) //length of datapackage in pfx
	#define TIMEOUT (32+DATA_PFX*3)
	#define INIT_OUT 100
#ifdef __cplusplus
	static constexpr char id_list[]="M1M2M3";
#endif

#elif SLOW_TX_MODE
	#define OUT_BUF_LEN 3000
	extern uint32_t out_buf[OUT_BUF_LEN]; //3k elems, or 12k bytes

#endif

#if STREAM
	#define UART_BUF_LEN 8192 //1024
	#define CCM_BUF_LEN 6144 //2048 for cur_val+=1
	#define SKIP 6*DEAD_INTERVAL*23
	extern int16_t uart_buf[UART_BUF_LEN]; //18k elems, or 36k bytes
	extern int16_t ccm_capture_buffer[CCM_BUF_LEN] __attribute__((section(".ccmram")));
#elif DECODE || DEBUG_TIM
	#define TWOPI 3.14159*2
#endif

#endif
