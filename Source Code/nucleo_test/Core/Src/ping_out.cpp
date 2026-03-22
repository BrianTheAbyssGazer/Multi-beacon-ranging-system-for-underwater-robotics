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


// global out buffer, with DMA to GPIO register
uint32_t out_buf[OUT_BUF_LEN];
static const std::string info="I am beacon 1!";


//initialize statics
volatile int PingOut::po_state = POState::PO_IDLE;
uint8_t PingOut::set_state = SetState::DISABLED;
volatile int PingOut::cur_out_pfx = 0;
volatile int PingOut::time_to_clear = 3;
volatile int PingOut::schedule_period = 0;
volatile bool PingOut::time_to_schedule_period = false;
volatile bool PingOut::time_to_schedule_databit = false;
volatile bool PingOut::time_to_schedule_phase_keying = false;
bool PingOut::debug = false;





PingOut :: PingOut(DMA_HandleTypeDef* p_hdma_tim2_up, TIM_HandleTypeDef* p_htim2, IndexInfoTX* p_index_info_tx) {

    /* assign callbacks */
    p_hdma_tim2_up->XferHalfCpltCallback = first_half_written_callback;
    p_hdma_tim2_up->XferCpltCallback =     secnd_half_written_callback;



    this->p_hdma_tim2_up = p_hdma_tim2_up;
    this->p_htim2 = p_htim2;
    this->p_index_info_tx = p_index_info_tx;

    /* parameter initialisation */
    peak_count = 4;
    samples_per_half_period = 6;
    /* schedule data initialisation */
    scheduled_idx = 0;
    clear_offset = 0;
    cur_idx=0;
    data_idx=0;
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
    if (time_to_schedule_period && periodic_schedule_enable){
    	time_to_schedule_period=false;
		scheduled_idx = 117;
		data_idx = 0;
		set_state=SetState::SET_PIN;
		cur_idx = scheduled_idx;
    }

    // set:
	switch (po_state)
	{
	case POState::FIRST_HLF_FREE:
		if (cur_idx<HAL_OUT_BUF_LEN) {
			set();
		}
		po_state=POState::PO_IDLE;
		break;

	case POState::SECND_HLF_FREE:
		if (cur_idx>=HAL_OUT_BUF_LEN) {
			set();
		}
		po_state=POState::PO_IDLE;
		break;
	case POState::PO_IDLE:
		break;
	}
}


// !! Side effects - sets schedule_idx and schedule_pfx to -1 after setting
//                 - sets clear_id to scheduled_idx;
//                 - sets time to clear to 0;
void PingOut::set() {
    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);}

	uint16_t i_set;         // Lower 9 bits (0-511)
	uint8_t i_bit,i_char;    // Next 3 bits (0-7)
	char c;
	bool bit;
	bool phase;
    while(1){
    	if((cur_idx>=HAL_OUT_BUF_LEN) && po_state==POState::FIRST_HLF_FREE){
    		break;
    	}
    	else if((cur_idx>=OUT_BUF_LEN) && po_state==POState::SECND_HLF_FREE){
    		cur_idx=cur_idx&OUT_BUF_MASK;
    		break;
    	}
		i_set = data_idx & (DEAD_INTERVAL-1);         // Lower 9 bits (0-511)
		i_bit  = (data_idx >> 9) & 7;    // Next 3 bits (0-7)
		i_char = data_idx >> 12;
		c = info[i_char];
		bit = c & 1 <<(i_bit);
		phase = bool(i_set & 1);
    	switch (set_state){
    	case SetState::CLEAR:
    		if(i_char>STRING_LEN){
    			set_state=SetState::DISABLED;
    			break;
    		}
    		if(i_set < N_CYCLE*2){
    			if(bit ^ phase)out_buf[cur_idx]=BSRR_PC6_RESET_MASK;
        		data_idx++;
            	cur_idx++;
    		}
    		else{
        		data_idx+=(DEAD_INTERVAL-N_CYCLE*2);
        		cur_idx+=(DEAD_INTERVAL-N_CYCLE*2);
    		}
    		break;
    	case SetState::SET_PIN:
    		if(i_char>STRING_LEN-1){
    			set_state=SetState::CLEAR;
    			data_idx-=OUT_BUF_LEN;
    			break;
    		}
    		if(i_set < N_CYCLE*2){
				if(phase ^ bit) out_buf[cur_idx]=BSRR_PC6_SET_MASK;
				else out_buf[cur_idx]=BSRR_PC6_RESET_MASK;
				data_idx++;
            	cur_idx++;
    		}
    		else{
        		data_idx+=(DEAD_INTERVAL-N_CYCLE*2);
        		cur_idx+=(DEAD_INTERVAL-N_CYCLE*2);
    		}
    		break;
    	case SetState::DISABLED:
    		cur_idx+=HAL_OUT_BUF_LEN;
    		break;
    	} //switch
    }//while
    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);}
}



void first_half_written_callback(DMA_HandleTypeDef *hdma) {

    if (PingOut::debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);}
    PingOut::cur_out_pfx = (PingOut::cur_out_pfx + 1)%(0x8000); //roll-over after 0x7FFF to match peak detector

    PingOut::po_state = POState::FIRST_HLF_FREE;

    if ((PingOut::cur_out_pfx%PingOut::schedule_period) == 0){
    	PingOut::time_to_schedule_period=true;
    }
}

void secnd_half_written_callback(DMA_HandleTypeDef *hdma) {
    if (PingOut::debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);}

    PingOut::po_state = POState::SECND_HLF_FREE;
}
