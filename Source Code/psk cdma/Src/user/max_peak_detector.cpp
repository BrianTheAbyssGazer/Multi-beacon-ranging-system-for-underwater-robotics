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
#include "arm_math.h"

#if BASIC_PEAK_DETECTOR_MODE || TRANSPONDER_MODE || TIME_OF_FLIGHT_MODE || ECHO_MASTER_MODE



// global ADC buffer:
int16_t buf[BUF_LEN];
#if STREAM
int16_t uart_buf[UART_BUF_LEN];
int16_t ccm_capture_buffer[CCM_BUF_LEN] __attribute__((section(".ccmram")));
#elif DECODE
int16_t buf_res[3]={1884,1884,1884};
#endif

//initialise statics:
volatile uint16_t MaxPeakDetector::global_state = MPDState::PROC_BUF_2ND_HLF;
volatile uint16_t MaxPeakDetector::cur_pfx = 0; // incremented each time the ADC buffer completely fills
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
    cur_idx = HAL_BUF_LEN;
    uart_idx = 0;
    ccm_idx = 0;
    enable_pfx=0;
    dead_zone_count = 0;
    signal_flag = false;
#if DECODE
    phase = 0;
    phase_int=0;
    sample_counter=0;
    symbol_counter=0;
    corr_sum=0;
#elif DEBUG_TIM
    pre_val=1884;
    corr_sum=0;
    sample_counter=0;
    symbol_counter=0;
#endif
	//zero initialise the adc buffer:
	for (uint16_t i = 0; i < BUF_LEN; i++) {
		buf[i] = 1884;
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
            if (cur_idx < HAL_BUF_LEN) {
                search_loop();
            } else {
                global_state = MPDState::IDLE;
            }
            break;

        case MPDState::PROC_BUF_2ND_HLF:
            if (cur_idx >= HAL_BUF_LEN) {
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
		switch (search_sub_state){
			case MPDSearchState::NO_SIGNAL: //------------------------------------------------------------------
				if (signal_flag) {
					uart_idx=0;
					ccm_idx=0;
					search_sub_state = MPDSearchState::YES_SIGNAL;
					signal_flag=false;
				}
				else cur_idx+=2;
				break;
			case MPDSearchState::YES_SIGNAL: //------------------------------------------------------------------
				if(ccm_idx<SKIP){
					ccm_idx+=DEAD_INTERVAL*6;
					cur_idx+=DEAD_INTERVAL*6;
				}
				else if (ccm_idx<CCM_BUF_LEN+SKIP){
					ccm_capture_buffer[ccm_idx-SKIP]=cur_val;
					ccm_idx++;
					cur_idx+=3;
				}
				else if(uart_idx<UART_BUF_LEN){
					uart_buf[uart_idx]=cur_val;
					uart_idx++;
					cur_idx+=3;
				}
				else{
					search_sub_state = MPDSearchState::SENDING;
				}
				break;
			case MPDSearchState::SENDING:
				(*p_index_info_tx).stream_adc(0);
				for (uint16_t i=0;i<CCM_BUF_LEN;i++)(*p_index_info_tx).stream_adc(ccm_capture_buffer[i]);
				for (uint16_t i=0;i<UART_BUF_LEN;i++)(*p_index_info_tx).stream_adc(uart_buf[i]);
				search_sub_state = MPDSearchState::SILENT;
			    last_peak_pfx=cur_pfx;
			    last_peak_idx=cur_idx;
				break;
			case MPDSearchState::SILENT:
				cur_idx++;
				if(cur_pfx>last_peak_pfx+14 && cur_idx>=last_peak_idx)search_sub_state = MPDSearchState::NO_SIGNAL;
				break;
		} // switch
		// conditions to escape search mode
		if ((global_state == MPDState::PROC_BUF_1ST_HLF) && (cur_idx >= (HAL_BUF_LEN))) {
			global_state = MPDState::IDLE;
#if TIME_OF_FLIGHT_MODE && DECODE
			for (uint8_t i=0;i<3;i++)buf_res[i]=buf[HAL_BUF_LEN-3+i];
#endif
			break;
		} else if (global_state == MPDState::PROC_BUF_2ND_HLF && (cur_idx >= (BUF_LEN))) {
			global_state = MPDState::IDLE;
			cur_idx = cur_idx % BUF_LEN;
#if TIME_OF_FLIGHT_MODE && DECODE
			for (uint8_t i=0;i<3;i++)buf_res[i]=buf[BUF_LEN-3+i];
#endif
			break;
		} else if (global_state == MPDState::ERROR_1) {
			break;
		}
	} //while

}

void MaxPeakDetector :: error_1_handle() {
    (*p_index_info_tx).send_byte(128); //send error notification
	global_state = MPDState::IDLE; // then skip this half buffer and try on the next

	//need to reset the relevant search context so we are ready to start searching again:
	search_sub_state = MPDSearchState::NO_SIGNAL;
}




//Called when first half of buffer is filled
extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* p_hadc) {
    MaxPeakDetector::cur_pfx = MaxPeakDetector::cur_pfx + 1; //roll-over after 0x7FFF to fit packet needs

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



Timestamp::Timestamp(uint16_t idx, uint16_t pfx) {
    this->idx = idx;
    this->pfx = pfx;
}

/*
* Returns the total timestamp integer from the Timestamp object
*/
uint16_t Timestamp::get_total(void) {
    return idx + BUF_LEN*pfx;
}

/*
* Returns a timestamp object from the total timestamp integer
*/
Timestamp Timestamp::from_total(uint16_t total_timestamp) {
    return Timestamp(total_timestamp % BUF_LEN, total_timestamp / BUF_LEN);
}

#endif
