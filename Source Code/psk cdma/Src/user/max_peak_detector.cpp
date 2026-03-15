/*
 * max_peak_detector.cpp
 *
 *  Created on: Sep 11, 2024
 *      Author: arthur
 */

//2 TODO slower pulse train on pico - weird delta is half expected ??? 20ms * 1.38 MHz = 27600, not observed 13800 which is half


#include "main.h"
#include "max_peak_detector.h"
#include "mode.h"
#include "global_buffer_def.h"

#if BASIC_PEAK_DETECTOR_MODE || TRANSPONDER_MODE || TIME_OF_FLIGHT_MODE || ECHO_MASTER_MODE



// global ADC buffer:
uint16_t buf[BUF_LEN];
#if STREAM
uint16_t uart_buf[UART_BUF_LEN];
uint16_t ccm_capture_buffer[CCM_BUF_LEN] __attribute__((section(".ccmram")));
#elif DECODE
uint16_t hilbert_buf[4]; //for Hilbert filter
#endif

//initialise statics:
volatile int MaxPeakDetector::global_state = MPDState::PROC_BUF_2ND_HLF;
volatile int MaxPeakDetector::cur_pfx = 0; // incremented each time the ADC buffer completely fills
/* Initialisation:
    // parameters
    min_aid = false;
    search_threshold_reduction = 5; 
    search_window = 200;
    dead_zone_len = 4000;
    search_threshold = 2500;
*/
MaxPeakDetector :: MaxPeakDetector(ADC_HandleTypeDef* p_hadc, TIM_HandleTypeDef* p_htim, IndexInfoTX* p_index_info_tx) {

    this->p_hadc = p_hadc;
    this->p_htim = p_htim;
    this->p_index_info_tx = p_index_info_tx;

	//initialise search sub-state:
	search_sub_state = MPDSearchState::NO_SIGNAL;

    //initialise start index of adc buffer (start half way through DMA will start at beginning):
    cur_idx = BUF_LEN/2;
    uart_idx = 0;
    ccm_idx = 0;
    bg_idx=0;
    bg_avg=0;

    //set default parameter values:
    min_aid = false;
	//initialise seach context:
	last_peak_val = 0; //TODO need calibration startup routine
    last_peak_idx = -1;
	last_peak_pfx = -1;
    tentative_max_val = 0;
    tentative_max_idx = 0;
	tentative_max_pfx = 0;
    tentative_min_val = 4096;
    tentative_min_idx = 0;
	tentative_min_pfx = 0;
    window_count = 0;
    dead_zone_count = 0;

	//zero initialise the adc buffer:
	for (int i = 0; i < BUF_LEN; i++) {
		buf[i] = 0;
	}

	//start ADC timer
	HAL_TIM_Base_Start(p_htim);
	// Start DMA
	HAL_ADC_Start_DMA(p_hadc, (uint32_t*)buf, BUF_LEN);
}



/*
* Returns under two conditions:
*   - A) a peak was detected
*   - B) the buffer half has been completely processed, waiting for new half
*
* A) Returns Timestamp struct with both fields set to -1 
* B) Returns the peak timestamp as Timestamp struct
*/
Timestamp MaxPeakDetector :: detect_peak() {
    //error_1_handle();
    //HAL_Delay(1000);
    switch (global_state)
    {
        case MPDState::PROC_BUF_1ST_HLF:
            if (cur_idx < BUF_LEN/2) {
                search_loop();
            } else {
                global_state = MPDState::IDLE;
            }
            break;

        case MPDState::PROC_BUF_2ND_HLF:
            if (cur_idx >= BUF_LEN/2) {
                search_loop();
            } else {
                global_state = MPDState::IDLE;
            }
            break;

        case MPDState::ERROR_1:
            error_1_handle();
            break;

        case MPDState::IDLE:
            break;
    }

    Timestamp tmsp(last_peak_idx, last_peak_pfx);
    return tmsp;
}

void MaxPeakDetector :: search_loop() {

	uint16_t cur_val;

	while (1) {
		cur_val = buf[cur_idx];
#if TIME_OF_FLIGHT_MODE && STREAM
		switch (search_sub_state){
			case MPDSearchState::NO_SIGNAL: //------------------------------------------------------------------
				if (cur_val > 2000||cur_val<1855) {
					uart_idx=0;
					ccm_idx=0;
					search_sub_state = MPDSearchState::YES_SIGNAL;
				}
				cur_idx++;
				break;
			case MPDSearchState::YES_SIGNAL: //------------------------------------------------------------------
				if(ccm_idx<SKIP){
					ccm_idx++;
					cur_idx++;
				}
				else if (ccm_idx<CCM_BUF_LEN+SKIP){
					ccm_capture_buffer[ccm_idx-SKIP]=cur_val;
					ccm_idx++;
					cur_idx++;
				}
				else if(uart_idx<UART_BUF_LEN){
					uart_buf[uart_idx]=cur_val;
					uart_idx++;
					cur_idx++;
				}
				else{
					search_sub_state = MPDSearchState::SENDING;
				}
				break;
			case MPDSearchState::SENDING:
				for (uint16_t i=0;i<CCM_BUF_LEN;i++)(*p_index_info_tx).stream_adc(ccm_capture_buffer[i]);
				for (uint16_t i=0;i<UART_BUF_LEN;i++)(*p_index_info_tx).stream_adc(uart_buf[i]);
				search_sub_state = MPDSearchState::NO_SIGNAL;
				break;
		} // switch
#elif TIME_OF_FLIGHT_MODE && DECODE
		switch (search_sub_state){
			case MPDSearchState::NO_SIGNAL: //------------------------------------------------------------------
				if (cur_val > 2000||cur_val<1855) {
					search_sub_state = MPDSearchState::YES_SIGNAL;
				}
				else {
					cur_idx+=11;
					break;
				}
			case MPDSearchState::YES_SIGNAL: //------------------------------------------------------------------

				break;
			case MPDSearchState::SENDING:

				break;
		} // switch
#endif
		// conditions to escape search mode
		if ((global_state == MPDState::PROC_BUF_1ST_HLF) && (cur_idx >= (BUF_LEN/2))) {
			global_state = MPDState::IDLE;
			break;
		} else if (global_state == MPDState::PROC_BUF_2ND_HLF && (cur_idx >= (BUF_LEN))) {
			global_state = MPDState::IDLE;
			cur_idx = 0;
			break;
		} else if (global_state == MPDState::ERROR_1) {
			break;
		}
	} //while

}
#if DECODE
uint16_t MaxPeakDetector :: shift(CircularBuffer *cb, uint16_t val) {
	cb->data[cb->head] = val;
	cb->head = (cb->head + 1) & HILB_MASK;
	return cb->data[cb->head];
}
#endif
void MaxPeakDetector :: error_1_handle() {
    (*p_index_info_tx).transmit_err_1(cur_idx, cur_pfx); //send error notification
	global_state = MPDState::IDLE; // then skip this half buffer and try on the next

    if (cur_idx > BUF_LEN/2) {
    	(*p_index_info_tx).stream_adc(18000);
    } else {
    	(*p_index_info_tx).stream_adc(9000);
    }

	//need to reset the relevant search context so we are ready to start searching again:
	search_sub_state = MPDSearchState::NO_SIGNAL;
}

 


//Called when first half of buffer is filled
extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* p_hadc) {
    MaxPeakDetector::cur_pfx = (MaxPeakDetector::cur_pfx + 1)%(0x8000); //roll-over after 0x7FFF to fit packet needs

	//state transition
		switch (MaxPeakDetector::global_state)
		  {
		  	case MPDState::PROC_BUF_1ST_HLF:
		  		MaxPeakDetector::global_state = MPDState::ERROR_1;
				break;
			case MPDState::PROC_BUF_2ND_HLF:
				MaxPeakDetector::global_state = MPDState::ERROR_1;
				break;

			case MPDState::ERROR_1:
				break;

			case MPDState::IDLE:
				MaxPeakDetector::global_state = MPDState::PROC_BUF_1ST_HLF;
				break;
		  }
}




//Called when buffer is completely filled
extern "C"  void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* p_hadc) {
    //state transition
		switch (MaxPeakDetector::global_state)
		  {
		  	case MPDState::PROC_BUF_1ST_HLF:
		  		MaxPeakDetector::global_state = MPDState::ERROR_1;
				break;
			case MPDState::PROC_BUF_2ND_HLF:
				MaxPeakDetector::global_state = MPDState::ERROR_1;
				break;
				
			case MPDState::ERROR_1:
				break;

			case MPDState::IDLE:
				MaxPeakDetector::global_state = MPDState::PROC_BUF_2ND_HLF;
				break;
		  }
}



Timestamp::Timestamp(int idx, int pfx) {
    this->idx = idx;
    this->pfx = pfx;
}

/*
* Returns the total timestamp integer from the Timestamp object
*/
int Timestamp::get_total(void) {
    return idx + BUF_LEN*pfx;
}

/*
* Returns a timestamp object from the total timestamp integer
*/
Timestamp Timestamp::from_total(int total_timestamp) {
    return Timestamp(total_timestamp % BUF_LEN, total_timestamp / BUF_LEN);
}

#endif
