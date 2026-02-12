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

#elif TRANSPONDER_MODE
	#define BUF_LEN 18000 
	extern uint16_t buf[BUF_LEN]; //18k elems, or 36k bytes
	#define OUT_BUF_LEN 3000 
	extern uint32_t out_buf[OUT_BUF_LEN]; //3k elems, or 12k bytes
	#define DATA_LEN 15

	//3k * 6 = 18k (6 ADC samples per half period of carrier)
	//36k bytes + 12k bytes = 48k bytes (will fit into memory)

#elif TIME_OF_FLIGHT_MODE
	#define BUF_LEN 18000 
	extern uint16_t buf[BUF_LEN]; //18k elems, or 36k bytes
	#define OUT_BUF_LEN 3000 
	extern uint32_t out_buf[OUT_BUF_LEN]; //3k elems, or 12k bytes
	#define DATA_LEN 15

	//3k * 6 = 18k (6 ADC samples per half period of carrier)
	//36k bytes + 12k bytes = 48k bytes (will fit into memory)
#elif ECHO_MASTER_MODE || ECHO_TRANSPONDER_MODE
	#define BUF_LEN 18000
	#define UART_BUF_LEN 2000
	#define BG_LEN 20
	extern uint16_t buf[BUF_LEN]; //18k elems, or 36k bytes
	extern uint16_t uart_buf[UART_BUF_LEN]; //18k elems, or 36k bytes
	extern uint16_t bg_buf[BG_LEN];
	#define OUT_BUF_LEN 3000
	extern uint32_t out_buf[OUT_BUF_LEN]; //3k elems, or 12k bytes
    #define DATA_LEN 8
#elif SLOW_TX_MODE
	#define OUT_BUF_LEN 3000
	extern uint32_t out_buf[OUT_BUF_LEN]; //3k elems, or 12k bytes

#endif


