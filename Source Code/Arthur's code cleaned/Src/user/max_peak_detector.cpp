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


#if BASIC_PEAK_DETECTOR_MODE || TRANSPONDER_MODE || TIME_OF_FLIGHT_MODE


// global ADC buffer:
uint16_t buf[BUF_LEN];


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
	search_sub_state = MPDSearchState::FIND_SIGNAL;

    //initialise start index of adc buffer (start half way through DMA will start at beginning):
    cur_idx = BUF_LEN/2;

    //set default parameter values:
    min_aid = false;
    search_threshold_reduction = 5; 
    search_window = 100;
    dead_zone_len = 3*(BUF_LEN/4);
    search_threshold = 2100;
    
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
    last_peak_idx = -1;
    last_peak_pfx = -1;
    return tmsp;
}


void MaxPeakDetector :: search_loop() {
    
	int cur_val;
    int peak_found = false;

	while (1) {
		cur_val = buf[cur_idx];

		switch (search_sub_state)
		{	
			case MPDSearchState::FIND_SIGNAL: //------------------------------------------------------------------
				if (cur_val > search_threshold) {
					search_sub_state = MPDSearchState::FIND_WINDOW_MAX;
				}

				cur_idx += 3;

				break;

			case MPDSearchState::FIND_WINDOW_MAX: //------------------------------------------------------------------
				window_count++;

				if (window_count > search_window) { // window is complete
					// we have a new peak! 
                    if (min_aid) {
                        Timestamp min_tmsp(tentative_min_idx, tentative_min_pfx);
                        Timestamp max_tmsp(tentative_max_idx, tentative_max_pfx);

                        //compute average timestamp:
                        Timestamp average_tmsp = Timestamp::from_total((min_tmsp.get_total() + max_tmsp.get_total())/2);
                        
                        last_peak_val = tentative_max_val;
                        last_peak_idx = average_tmsp.idx;
                        last_peak_pfx = average_tmsp.pfx;

                    } else {
                        last_peak_val = tentative_max_val;
                        last_peak_idx = tentative_max_idx;
                        last_peak_pfx = tentative_max_pfx;
                    }
                    search_threshold = last_peak_val * 0.85;
                    (*p_index_info_tx).transmit_idx(last_peak_idx, last_peak_pfx, last_peak_val);
					
					//reset for next peak:
					tentative_max_val = 0;
					tentative_max_idx = 0;
					tentative_max_pfx = 0;
                    tentative_min_val = 4096;
					tentative_min_idx = 0;
					tentative_min_pfx = 0;

					//update search_threshold TODO switched off for range measurements as echos are dominating
					//search_threshold = last_peak_val - (last_peak_val/search_threshold_reduction);

					//move to DEAD_ZONE
					window_count = 0;
					search_sub_state = MPDSearchState::DEAD_ZONE;
					
				} else if (cur_val > tentative_max_val) { //new max
					tentative_max_val = cur_val;
					tentative_max_idx = cur_idx;
					tentative_max_pfx = cur_pfx;

				} else if (cur_val < tentative_min_val) { //new min
                    tentative_min_val = cur_val;
					tentative_min_idx = cur_idx;
					tentative_min_pfx = cur_pfx;
				}

				cur_idx++;
				break;

			case MPDSearchState::DEAD_ZONE: //------------------------------------------------------------------
				
                if (dead_zone_len > 0) {
                    //leap over dead_zone:
                    cur_idx = (cur_idx + dead_zone_len) % BUF_LEN;

                } else {
                    //just jump to end of buffer:
                    cur_idx = BUF_LEN;
                }

                search_sub_state = MPDSearchState::FIND_SIGNAL;
                peak_found = true;
				break;
							
			
			//------------------------------------------------------------------
				
		} // switch

		

		// conditions to escape search mode
        if (peak_found) { // we found a peak and need to terminate
            peak_found = false;
            global_state = MPDState::IDLE;
			break;
		} else if ((global_state == MPDState::PROC_BUF_1ST_HLF) && (cur_idx >= (BUF_LEN/2))) {
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

void MaxPeakDetector :: error_1_handle() {
    (*p_index_info_tx).transmit_err_1(cur_idx, cur_pfx); //send error notification
	global_state = MPDState::IDLE; // then skip this half buffer and try on the next

    if (cur_idx > BUF_LEN/2) {
        cur_idx = BUF_LEN/2;
    } else {
        cur_idx = 0;
    }

	//need to reset the relevant search context so we are ready to start searching again:
	search_sub_state = MPDSearchState::FIND_SIGNAL;

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
