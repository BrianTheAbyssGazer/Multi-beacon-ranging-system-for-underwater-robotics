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

#if  TRANSPONDER_MODE || TIME_OF_FLIGHT_MODE


// global out buffer, with DMA to GPIO register
uint32_t out_buf[OUT_BUF_LEN];



//initialize statics
volatile int PingOut::po_state = POState::SECND_HLF_FREE;
volatile int PingOut::cur_out_pfx = 0;
volatile int PingOut::time_to_clear = 3;
volatile int PingOut::schedule_period = 0;
volatile uint8_t PingOut::datapacket_index = -1;
bool PingOut::codeword[DATA_LEN]={1,0,1,0,0,1,1,1};
//bool PingOut::codeword[DATA_LEN]={1,1,1,1};
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
    set_half = true;
    clear_idx = -1;
    data_idx = 0;
    sending=false;
    periodic_schedule_enable = false;

    /* buffer initialization to reset (LOW):*/
	for (int i = 0; i < OUT_BUF_LEN; i++) {
		out_buf[i] = BSRR_PC6_RESET_MASK;
	}
	//for (int i = 600; i < 3000; i+=2) {
	//	out_buf[i] = BSRR_PC6_SET_MASK;
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
    if (PingOut::time_to_schedule_period){
    	//periodic scheduling:
        if (periodic_schedule_enable) {
        	scheduled_idx = 0;
        	clear_offset = 0;
        	data_idx = 0;
            clear_idx =0;
            time_to_clear=0;
            PingOut::time_to_schedule_period = false;
            sending = true;
            set_half = true;
        }
    }

    // clear:
    if (time_to_clear==2) {
        clear(clear_offset);
    }

    // set:
    if (sending) {
        switch (PingOut::po_state)
        {
        case POState::FIRST_HLF_FREE:
            if (scheduled_idx<(OUT_BUF_LEN/2)) {
                set(scheduled_idx);
                set_half = false;
            }
            break;

        case POState::SECND_HLF_FREE:
            if (scheduled_idx>=(OUT_BUF_LEN/2)) {
                set(scheduled_idx);
                set_half = true;
            }
            break;
        }
    }
}


// !! Side effects - sets schedule_idx and schedule_pfx to -1 after setting
//                 - sets clear_id to scheduled_idx;
//                 - sets time to clear to 0;
void PingOut::set(uint16_t offset) {
    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);}
    uint16_t imax;
    if (data_idx+30>=DATA_LEN*8){
    	sending=false;
    	imax=DATA_LEN*8;
    }
    else imax=data_idx+30;
    for (uint16_t i=0;data_idx<imax;data_idx++,i++){
    	if(data_idx%8==0){
    		if(codeword[data_idx/8]){
    			for(uint16_t j=0; j<N_CYCLE*2;j+=2) out_buf[offset+i*N_CYCLE*2+j]=BSRR_PC6_SET_MASK;
    		}
    		else{
    			for(uint16_t j=1; j<N_CYCLE*2;j+=2) out_buf[offset+i*N_CYCLE*2+j]=BSRR_PC6_SET_MASK;
    		}
    	}
    }
    scheduled_idx=(scheduled_idx+OUT_BUF_LEN/2)%OUT_BUF_LEN;
    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);}
}

// !! Side effect - sets clear_idx to -1 after clearing
void PingOut::clear(uint16_t offset) {
	if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);}
    uint16_t imax;
    if (clear_idx+30>=DATA_LEN*8){
    	imax=DATA_LEN*8;
    	time_to_clear=3;
    }
    else {
    	imax=clear_idx+30;
    	time_to_clear=1;
    }
    for (uint16_t i=0;clear_idx<imax;clear_idx++,i++){
    	if(data_idx%8==0){
    		if(codeword[clear_idx/8]){
    			for(uint16_t j=0; j<N_CYCLE*2;j+=2) out_buf[offset+i*N_CYCLE*2+j]=BSRR_PC6_RESET_MASK;
    		}
    		else{
    			for(uint16_t j=1; j<N_CYCLE*2;j+=2) out_buf[offset+i*N_CYCLE*2+j]=BSRR_PC6_RESET_MASK;
    		}
    	}
    }
    for (uint16_t i=0;i<OUT_BUF_LEN/2;i++){
    	out_buf[i+clear_offset]=BSRR_PC6_RESET_MASK;
    }
    clear_offset=(clear_offset+OUT_BUF_LEN/2)%OUT_BUF_LEN;

    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);}
}




void first_half_written_callback(DMA_HandleTypeDef *hdma) {

    if (PingOut::debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);}
    PingOut::cur_out_pfx = (PingOut::cur_out_pfx + 1)%(0x8000); //roll-over after 0x7FFF to match peak detector
    PingOut::po_state = POState::FIRST_HLF_FREE;
    if (PingOut::time_to_clear < 2) {
        PingOut::time_to_clear++;
    }

    //periodic scheduling
    if ((PingOut::cur_out_pfx%PingOut::schedule_period) == 0)
    {
        PingOut::time_to_schedule_period = true;
    }
}

void secnd_half_written_callback(DMA_HandleTypeDef *hdma) {
    if (PingOut::debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);}
    PingOut::po_state = POState::SECND_HLF_FREE;
    if (PingOut::time_to_clear < 2) {
        PingOut::time_to_clear++;
    }

}


#endif
