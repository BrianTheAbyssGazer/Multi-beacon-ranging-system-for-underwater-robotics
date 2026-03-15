/*
 * global_buffer_def.h
 *
 *  Created on: Aug 14, 2024
 *      Author: arthur
 */

#include "mode.h"

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
	#define BUF_LEN 16384 // 16384 = 2^14
	#define HAL_BUF_LEN 8192 // 16384 = 2^14
	#define BUF_MASK 16383
	#define HAL_BUF_MASK 8191
	extern uint16_t buf[BUF_LEN]; //18k elems, or 36k bytes
	#define OUT_BUF_LEN 2048 // 2048 = 2^11
	#define OUT_BUF_MASK 2047
	extern uint32_t out_buf[OUT_BUF_LEN]; //3k elems, or 12k bytes
    #define DATA_LEN 8
	#define N_CYCLE 16 //cycle per symbol

#elif SLOW_TX_MODE
	#define OUT_BUF_LEN 3000
	extern uint32_t out_buf[OUT_BUF_LEN]; //3k elems, or 12k bytes

#endif

#if STREAM
	#define UART_BUF_LEN 1500
	#define CCM_BUF_LEN 4096
	#define SKIP 0
	extern uint16_t uart_buf[UART_BUF_LEN]; //18k elems, or 36k bytes
	extern uint16_t ccm_capture_buffer[CCM_BUF_LEN] __attribute__((section(".ccmram")));
#elif DECODE
	#define HILB_SIZE 4
	#define HILB_MASK (HILB_SIZE-1)
	#define DEAD_ZONE_LEN 8191
	#define TWOPI 3.14159*2
#endif
