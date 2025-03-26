/*
 * basic_pulse_stream.c
 *
 *  Created on: Aug 14, 2024
 *      Author: arthur
 */



#include "main.h"
#include "basic_pulse_stream.h"
#include "stm32f303xe.h"
#include "mode.h"

#if BASIC_PULSE_STREAM_MODE

uint32_t small_buff[SMALL_BUFF_LEN];

const uint32_t set_mask = BSRR_PC6_SET_MASK;
const uint32_t reset_mask = BSRR_PC6_RESET_MASK;

void basic_pulse_stream_init(DMA_HandleTypeDef* p_hdma_tim2_up, TIM_HandleTypeDef* p_htim2){

	/* buffer initialization */

	//start with setting buffer default to reset (LOW):
	for (int i = 0; i < SMALL_BUFF_LEN; i++) {
		small_buff[i] = reset_mask;
	}


	//envelope on for first pulse:
	for (int i = 0; i < (PULSE_LEN*2); i+=2){
		small_buff[i] = set_mask;
	}

	//then five more pulses:
	for (int i = (PULSE_PERIOD * 2); i < ((PULSE_PERIOD *2) + (PULSE_LEN*2)); i+=2){
		small_buff[i] = set_mask;
	}


	/*
	 * initialize the DMA and Timer
	 */

	// DMA, circular memory to peripheral mode, full word (32 bit) transfer
	HAL_DMA_Start(p_hdma_tim2_up, (uint32_t)small_buff, (uint32_t)&(GPIOC->BSRR), SMALL_BUFF_LEN);
	HAL_TIM_Base_Start(p_htim2);
	TIM2->DIER |= (1<<8); //set UDE bit (update dma request enable)

}



void basic_pulse_stream_main() {
	while (1) {

	}
}


#endif
