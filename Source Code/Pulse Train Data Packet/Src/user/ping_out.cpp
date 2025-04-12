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

#if  TRANSPONDER_MODE || TIME_OF_FLIGHT_MODE ||SLOW_TX_MODE

// global out buffer, with DMA to GPIO register
uint32_t out_buf[OUT_BUF_LEN];



//initialize statics
volatile int PingOut::po_state = POState::SECND_HLF_FREE;
volatile int PingOut::cur_out_pfx = 0;
volatile int PingOut::time_to_clear = 2;
volatile int PingOut::schedule_period = 0;
volatile uint8_t PingOut::datapacket_index = -1;
bool PingOut::codeword[16];
volatile bool PingOut::time_to_schedule_period = false;
volatile bool PingOut::time_to_schedule_databit = false;
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
    scheduled_idx = -1;
    scheduled_pfx = -1;
    clear_idx = -1;

    periodic_schedule_enable = false;
    codeword[0]=true;
    codeword[15]=true;

    /* buffer initialization to reset (LOW):*/
	for (int i = 0; i < OUT_BUF_LEN; i++) {
		out_buf[i] = BSRR_PC6_RESET_MASK;
	}


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
void PingOut::schedule_ping(int idx, int pfx) {
    scheduled_idx = idx/samples_per_half_period;
    scheduled_pfx = pfx;
}

bool PingOut::calculateParity(bool codeword[], const uint8_t positions[], uint8_t size) {
	bool parity = 0;
	for (uint8_t i = 0; i < size; ++i) {
		parity ^= codeword[positions[i]]; // 1-based to 0-based
	}
	return parity;
}

// Start datapacket scheduling
void PingOut::start_datapacket_scheduler(uint8_t data) {
	datapacket_index = 0;
	bool dataBits[8];
	for (int i = 0; i < 8; ++i) {
		dataBits[7 - i] = (data >> i) & 1; // MSB first
	}
	uint8_t dataPos[] = {3, 5, 6, 7, 9, 10, 11, 12};
	for (uint8_t i = 0; i < 8; ++i) {
		codeword[dataPos[i]] = dataBits[i];
	}
	// Define parity positions
	const uint8_t p1_pos[] = {1, 3, 5, 7, 9, 11, 13};
	const uint8_t p2_pos[] = {2, 3, 6, 7, 10, 11, 14};
	const uint8_t p4_pos[] = {4, 5, 6, 7, 12, 13, 14};
	const uint8_t p8_pos[] = {8, 9, 10, 11, 12, 13, 14};
    // Calculate and set Hamming parity bits
    codeword[0]  = calculateParity(codeword, p1_pos, 7);  // P1 at pos 1
    codeword[1]  = calculateParity(codeword, p2_pos, 7);  // P2 at pos 2
    codeword[3]  = calculateParity(codeword, p4_pos, 7);  // P4 at pos 4
    codeword[7]  = calculateParity(codeword, p8_pos, 7);  // P8 at pos 8
    bool overall_parity = 0;
	for (uint8_t i = 1; i < 14; ++i) {
		overall_parity ^= codeword[i];
	}
	codeword[14] = overall_parity;
}

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
            scheduled_pfx = PingOut::cur_out_pfx;
            PingOut::time_to_schedule_period = false;
        }
    }
    else if(PingOut::time_to_schedule_databit){
    	scheduled_idx = 0;
    	scheduled_pfx = PingOut::cur_out_pfx;
    	PingOut::time_to_schedule_databit = false;
    	time_to_clear = 2;
    }


    // clear:
    if (clear_idx >= 0 && time_to_clear == 2) {
        clear(clear_idx);
    }
    
    // set:
    if (PingOut::cur_out_pfx == scheduled_pfx) {
        switch (PingOut::po_state)
        {
        case POState::FIRST_HLF_FREE:
            if (scheduled_idx < (OUT_BUF_LEN/2)) {
                set(scheduled_idx);
            }
            break;
        
        case POState::SECND_HLF_FREE:
            if (scheduled_idx >= (OUT_BUF_LEN/2)) {
                set(scheduled_idx);
            }
            break;
        }     
    }


    
        
}


// !! Side effects - sets schedule_idx and schedule_pfx to -1 after setting
//                 - sets clear_id to scheduled_idx;
//                 - sets time to clear to 0;
void PingOut::set(int out_idx) {
    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);}
    

    //Need small delay before setting in case we are setting right at the
    //temporal start of a half-buffer, and the width of the pulse envelope will 
    // go over the half buffer boundary.
    // The processing delay from the ADC data should be sufficient.
   

    for (int i = 0; i < peak_count*2; i = (i+2)%(OUT_BUF_LEN)) {
        out_buf[out_idx + i] = BSRR_PC6_SET_MASK;
    }

    clear_idx = scheduled_idx;
    time_to_clear = 0;

    scheduled_idx = -1;
    scheduled_pfx = -1;

    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);} 
    
}

// !! Side effect - sets clear_idx to -1 after clearing
void PingOut::clear(int out_idx) {
    if (debug) {HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);} 
    for (int i = 0; i < peak_count*2; i = (i+2)%(OUT_BUF_LEN)) {
        out_buf[out_idx + i] = BSRR_PC6_RESET_MASK;
    }

    clear_idx = -1;
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
    
    //datapacket impulses scheduling
    if(PingOut::datapacket_index>-1 && PingOut::datapacket_index<15){
    	if(PingOut::datapacket_index==0) {
			PingOut::time_to_schedule_databit = true;
		}
		else if (PingOut::codeword[PingOut::datapacket_index]){
			PingOut::time_to_schedule_databit = true;
		}
    	PingOut::datapacket_index+=1;
    }
	else if(PingOut::datapacket_index==15){
		PingOut::time_to_schedule_databit = true;
		PingOut::datapacket_index=-1;
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
