/*
 * ping_out.cpp
 *
 *  Created on: Sep 13, 2024
 *      Author: arthur
 */

#include "main.h"
#include "max_peak_detector.h"
#include "mode.h"
#include "global_buffer_def.h"
#include "ping_out.h"
#include <string>
#if  TRANSPONDER_MODE || TIME_OF_FLIGHT_MODE


// global out buffer, with DMA to GPIO register
uint32_t out_buf[OUT_BUF_LEN];
static const std::string info="I am beacon 1!";


//initialize statics
volatile int PingOut::po_state = POState::SECND_HLF_FREE;
volatile int PingOut::cur_out_pfx = 0;
volatile int PingOut::time_to_clear = 3;
volatile int PingOut::schedule_period = 0;
volatile bool PingOut::time_to_schedule_period = false;
volatile bool PingOut::time_to_schedule_databit = false;
volatile bool PingOut::time_to_schedule_phase_keying = false;
bool PingOut::debug = false;





PingOut :: PingOut(DMA_HandleTypeDef* p_hdma_tim2_up, TIM_HandleTypeDef* p_htim2) {

    /* assign callbacks */
    p_hdma_tim2_up->XferHalfCpltCallback = first_half_written_callback;
    p_hdma_tim2_up->XferCpltCallback =     secnd_half_written_callback;



    this->p_hdma_tim2_up = p_hdma_tim2_up;
    this->p_htim2 = p_htim2;

    /* parameter initialisation */
    peak_count = 4;
    samples_per_half_period = 6;
    /* schedule data initialisation */
    scheduled_idx = 0;
    clear_offset = 0;
    cur_idx=0;
    data_idx=0;
    set_state=SetState::IDLE;
    periodic_schedule_enable = false;

    /* buffer initialization to reset (LOW):*/
	for (int i = 0; i < OUT_BUF_LEN; i++) {
		out_buf[i] = BSRR_PC6_RESET_MASK;
	}
	//for (int i = 0; i < 8; i++) {
	//	for (int j = 0; j<16; j+=2) out_buf[i*DEAD_INTERVAL+j] = BSRR_PC6_SET_MASK;
	//}


    /* initialize the DMA and Timer */

	/* DMA, circular memory to peripheral mode,
        full word (32 bit) transfer */
	HAL_DMA_Start_IT(p_hdma_tim2_up,
                    (uint32_t)out_buf,
                    (uint32_t)&(GPIOC->BSRR),
                    OUT_BUF_LEN);

    HAL_TIM_Base_Start(p_htim2);
	TIM2->DIER |= (1<<8); //set UDE bit (update dma request enable)

}


/*
* Schedule output at ADC buffer index idx, and prefix pfx.
* Any un-transmitted scheduled output will be overwritten.
* Do not call more than once per total buffer.
* idx will be rounded for the Out buffer in this function.
* pfx is the same for both in and out buffer as they are the same temporal length.
*/

/*
* Start periodic scheduling, with period in units of total buffer length
*/
void PingOut::start_periodic_scheduler(int period) {
    PingOut::schedule_period = period;
    periodic_schedule_enable = true;
}

/*
* Should be run at least twice per full in/out buffer (one per half),
* to ensure any scheduled output is actually transmitted.
*/
void PingOut::update() {
    if ((PingOut::cur_out_pfx%PingOut::schedule_period) == 0 && periodic_schedule_enable){
		scheduled_idx = 0;
		data_idx = 0;
		time_to_clear=0;
		set_state==SetState::SET;
    }

    // set:
	switch (po_state)
	{
	case POState::FIRST_HLF_FREE:
		if (cur_idx<HAL_OUT_BUF_LEN) {
			if (set_state==SetState::SET) set();
			else if(set_state==SetState::CLEAR) clear();
		}
		po_state=POState::IDLE;
		break;

	case POState::SECND_HLF_FREE:
		if (cur_idx>=HAL_OUT_BUF_LEN) {
			if (set_state==SetState::SET) set();
			else if(set_state==SetState::CLEAR) clear();
		}
		po_state=POState::IDLE;
		break;
	case POState::IDLE:
		break;
	}
}


// !! Side effects - sets schedule_idx and schedule_pfx to -1 after setting
//                 - sets clear_id to scheduled_idx;
//                 - sets time to clear to 0;
void PingOut::set(uint16_t offset) {
    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);}
#if DEBUG_TIM
    for (uint8_t i=0;i<4;i++,data_idx++){
    	if(data_idx<800)for(uint16_t j=0; j<16;j+=2) out_buf[scheduled_idx+i*DEAD_INTERVAL+j]=BSRR_PC6_SET_MASK;
    	else sending=false;
    }
#else
	uint16_t i_set;         // Lower 9 bits (0-511)
	uint8_t i_bit,i_char;    // Next 3 bits (0-7)
    while(1){
    	if((cur_idx>=HAL_OUT_BUF_LEN) && po_state==POState::FIRST_HLF_FREE){
    		break;
    	}
    	else if((cur_idx>=OUT_BUF_LEN) && po_state==POState::SECND_HLF_FREE){
    		cur_idx=cur_idx&OUT_BUF_MASK;
    		break;
    	}
    	else{

    	}

		i_set = data_idx & 511;         // Lower 9 bits (0-511)
		i_bit  = (data_idx >> 9) & 7;    // Next 3 bits (0-7)
		i_char = data_idx >> 12;
		if(i_char>13){
			set_state==SetState::CLEAR;
			time_to_clear=2;
			break;
		}
		char c = info[i_char];
		bool bit = c & 1 <<(i_bit);
		bool phase = bool(i_set & 1);
		if(bit){
			if(i_set < N_CYCLE*2 && (!phase)) out_buf[cur_idx]=BSRR_PC6_SET_MASK;
			else out_buf[cur_idx]=BSRR_PC6_RESET_MASK;
		}
		else{
			if(i_set < N_CYCLE*2 && phase) out_buf[cur_idx]=BSRR_PC6_SET_MASK;
			else out_buf[cur_idx]=BSRR_PC6_RESET_MASK;
		}
		data_idx++;
    	cur_idx++;
    }
    po_state = POState::IDLE;
#endif
    scheduled_idx=(scheduled_idx+HAL_OUT_BUF_LEN)&OUT_BUF_MASK;

    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);}
}

// !! Side effect - sets clear_idx to -1 after clearing
void PingOut::clear(uint16_t offset) {
	if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);}
	time_to_clear=1;
#if DEBUG_TIM
    for (uint8_t i=0;i<4;i++,clear_idx++){
    	if(clear_idx<800) for(uint16_t j=0; j<16;j+=2) out_buf[offset+i*DEAD_INTERVAL+j]=BSRR_PC6_RESET_MASK;
    	else time_to_clear=3;
    }
#else
	for (uint8_t i=0;i<HAL_OUT_BUF_LEN/DEAD_INTERVAL;i++,clear_idx++){
    	if(clear_idx<DATA_LEN){
    		size_t i_char = clear_idx / 8; // Determine which character
    		size_t i_bit = clear_idx & 7; // Determine which bit (0-7)
    		unsigned char character = static_cast<unsigned char>(info[i_char]);
    		bool bit=(character >> i_bit) & 1;
    		if(bit)for(uint16_t j=0; j<N_CYCLE*2;j+=2) out_buf[clear_offset+i*DEAD_INTERVAL+j]=BSRR_PC6_RESET_MASK;
    		else for(uint16_t j=1; j<N_CYCLE*2;j+=2) out_buf[clear_offset+i*DEAD_INTERVAL+j]=BSRR_PC6_RESET_MASK;
    	}
    	else{
    		time_to_clear=3;
    	}
    }
#endif
    clear_offset=(clear_offset+HAL_OUT_BUF_LEN)&OUT_BUF_MASK;

    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);}
}




void first_half_written_callback(DMA_HandleTypeDef *hdma) {

    if (PingOut::debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);}
    PingOut::cur_out_pfx = (PingOut::cur_out_pfx + 1)%(0x8000); //roll-over after 0x7FFF to match peak detector
    if (PingOut::time_to_clear < 2) {
        PingOut::time_to_clear++;
    }

    //periodic scheduling
    if (PingOut::po_state = POState::IDLE)
    {
        PingOut::po_state = POState::FIRST_HLF_FREE;
    }
    else PingOut::po_state = POState::ERROR;
}

void secnd_half_written_callback(DMA_HandleTypeDef *hdma) {
    if (PingOut::debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);}

    if (PingOut::time_to_clear < 2) {
        PingOut::time_to_clear++;
    }

    //periodic scheduling
    if (PingOut::po_state = POState::IDLE)
    {
    	PingOut::po_state = POState::SECND_HLF_FREE;
    }
    else PingOut::po_state = POState::ERROR;
}


#endif
