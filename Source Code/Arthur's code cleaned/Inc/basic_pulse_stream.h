/*
 * basic_pulse_stream.h
 *
 *  Created on: Aug 14, 2024
 *      Author: arthur
 */

#include "main.h"


/*
 * Carrier frequency is 115kHz
 */

#define PULSE_LEN 8 // units: periods of 115kHz carrier
#define PULSE_PERIOD 500 // units: periods of 115kHz carrier

#define SMALL_BUFF_LEN (PULSE_PERIOD*4)
extern uint32_t small_buff[SMALL_BUFF_LEN];


#define BSRR_PC6_SET_MASK 1<<6
#define BSRR_PC6_RESET_MASK 1<<22



void basic_pulse_stream_init(DMA_HandleTypeDef*, TIM_HandleTypeDef*);

void basic_pulse_stream_main(void);

