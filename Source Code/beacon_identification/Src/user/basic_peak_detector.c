/*
 * basic_peak_detector.c
 *
 *  Created on: Aug 22, 2024
 *      Author: arthur
 */

#include <stdlib.h>
#include "main.h"
#include "basic_peak_detector.h"
#include "index_info_transmit.h"
#include "mode.h"
#include "global_buffer_def.h"


//#if BASIC_PEAK_DETECTOR_MODE
#if 0

void peak_search(void);
void error_1_handle(void);

/**************** ADC BUFFER *******************/
uint16_t buf[BUF_LEN];
/***********************************************/


/******************* STATES ***********************/
#define IDLE_STATE 0
#define PROC_BUF_1ST_HLF_STATE 1
#define PROC_BUF_2ND_HLF_STATE 2
#define ERROR_1_STATE 3
volatile int global_state;
/******************************************************/


/****************** PEAK SEARCH SUB-STATES ********************/
// Parameters:
#define SEARCH_THRESHOLD_REDUCTION 5 
#define SEARCH_WINDOW 200 // samples (this is tight)
#define DEAD_ZONE 4000 //samples 
#define AVERAGE_SENSITIVITY 2000 //looks good experimentally
#define SEARCH_BLOCK 6
#define SEARCH_FORWARD_JUMP 48 //8 x 6
#define SEARCH_BACK_JUMP 96 //2 x 48

// Search sub-state enum:
#define SEARCH_STATE_FIND_SIGNAL_SPARSE 0
#define SEARCH_STATE_FIND_SIGNAL_START 1
#define SEARCH_STATE_FIND_WINDOW_MAX 2
#define SEARCH_STATE_DEAD_ZONE 3
int search_sub_state;

//search context:
int last_peak_val; // the value of the last successfully detected pulse peak
int last_peak_idx;
int last_peak_pfx;
int tentative_max_val;
int tentative_max_idx;
int tentative_max_pfx;
int search_count;
int window_count;
int dead_zone_count;
float running_average; // the average signal value, updated continuously except when searching
int search_threshold;

int cur_idx; //current idx of adc buffer
volatile int cur_pfx; // incremented each time the buffer completely fills

/*******************************************************************/


void basic_peak_detector_init(ADC_HandleTypeDef* p_hadc, TIM_HandleTypeDef* p_htim, UART_HandleTypeDef* p_huart) {
	//initialise global state:
	global_state = PROC_BUF_2ND_HLF_STATE;

	//initialise start index of adc buffer (start half way through DMA will start at beginning):
	cur_idx = BUF_LEN/2;
	cur_pfx = 0;

	//initialise search sub-state:
	search_sub_state = SEARCH_STATE_FIND_SIGNAL_SPARSE;

	//initialise seach context:
	last_peak_val = 500; //TODO need calibration startup routine
    last_peak_idx = 0;
	last_peak_pfx = 0;
    tentative_max_val = 0;
    tentative_max_idx = 0;
	tentative_max_pfx = 0;
	search_count = 0;
    window_count = 0;
    dead_zone_count = 0;
    running_average = 0.0; //set to 1900 for fish tank
    search_threshold = last_peak_val - (last_peak_val/SEARCH_THRESHOLD_REDUCTION);


	//zero initialise the adc buffer:
	for (int i = 0; i < BUF_LEN; i++) {
		buf[i] = 0;
	}

	//initialise comms:
	index_info_transmit_init(p_huart);

	//start ADC timer
	HAL_TIM_Base_Start(p_htim);
	// Start DMA
	HAL_ADC_Start_DMA(p_hadc, (uint32_t*)buf, BUF_LEN);

}






void basic_peak_detector_main() {
	while (1)
	  {
		//error_1_handle();
		//HAL_Delay(1000);
		  switch (global_state)
		  {
		  	case PROC_BUF_1ST_HLF_STATE:
				peak_search();
				break;
			
			case PROC_BUF_2ND_HLF_STATE:
				peak_search();
				break;

			case ERROR_1_STATE:
				error_1_handle();
				break;

			case IDLE_STATE:
				break;

		  	default:
				global_state = IDLE_STATE;
				break;
		  }
	  }



}



//Called when first half of buffer is filled
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* p_hadc) {
	cur_pfx = (cur_pfx + 1)%(0x1000); //roll-over at 0x0FFF to fit packet needs

	//state transition
		switch (global_state)
		  {
		  	case PROC_BUF_1ST_HLF_STATE:
				global_state = ERROR_1_STATE;
				break;
			case PROC_BUF_2ND_HLF_STATE:
				global_state = ERROR_1_STATE;
				break;

			case ERROR_1_STATE:
				break;

			case IDLE_STATE:
				global_state = PROC_BUF_1ST_HLF_STATE;
				break;

		  	default:
				global_state = PROC_BUF_1ST_HLF_STATE;
				break;
		  }
}


//Called when buffer is completely filled
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* p_hadc) {
	//state transition
		switch (global_state)
		  {
		  	case PROC_BUF_1ST_HLF_STATE:
				global_state = ERROR_1_STATE;
				break;
			case PROC_BUF_2ND_HLF_STATE:
				global_state = ERROR_1_STATE;
				break;
				
			case ERROR_1_STATE:
				break;

			case IDLE_STATE:
				global_state = PROC_BUF_2ND_HLF_STATE;
				break;

		  	default:
				global_state = PROC_BUF_2ND_HLF_STATE;
				break;
		  }
}




void error_1_handle() {
	transmit_err_1(cur_idx, cur_pfx); //send error notification
	global_state = IDLE_STATE; // then skip this half buffer and try on the next

	//need to reset the relevant search context so we are ready to start searching again:
	search_sub_state = SEARCH_STATE_FIND_SIGNAL_SPARSE;
	last_peak_idx = 0;
    tentative_max_val = 0;
    tentative_max_idx = 0;
	search_count = 0;
    window_count = 0;
    dead_zone_count = 0;
	
}


void peak_search() {

	int cur_val;

	while (1) {
		cur_val = buf[cur_idx];

		// update running average of signal, center about zero then take magnitude:
		//running_average = ((running_average * (AVERAGE_SENSITIVITY-1) + (float) cur_val)/AVERAGE_SENSITIVITY);
		//cur_val = abs(cur_val - (int)(running_average));

		switch (search_sub_state)
		{	
			case SEARCH_STATE_FIND_SIGNAL_SPARSE: //------------------------------------------------------------------
				/*
				search_count++;
				
				if ( cur_idx < SEARCH_BACK_JUMP || cur_idx > (BUF_LEN - SEARCH_BACK_JUMP) ) {
					search_count = 0;
					if (cur_val > search_threshold) {
						search_sub_state = SEARCH_STATE_FIND_WINDOW_MAX;
					} 

				} else if (cur_val > search_threshold) {
					search_count = 0;
					cur_idx -= SEARCH_BACK_JUMP;
					search_sub_state = SEARCH_STATE_FIND_SIGNAL_START;

				} else if (search_count > SEARCH_BLOCK) {
					search_count = 0;
					cur_idx = (cur_idx + SEARCH_FORWARD_JUMP) % (BUF_LEN - SEARCH_BACK_JUMP);
					search_sub_state = SEARCH_STATE_FIND_SIGNAL_START;
				
				} 
				*/

				search_sub_state = SEARCH_STATE_FIND_SIGNAL_START;
				
				break;

			case SEARCH_STATE_FIND_SIGNAL_START: //------------------------------------------------------------------
				if (cur_val > search_threshold) {
					search_sub_state = SEARCH_STATE_FIND_WINDOW_MAX;

				}

				cur_idx += 2;

				break;

			case SEARCH_STATE_FIND_WINDOW_MAX: //------------------------------------------------------------------
				window_count++;

				if (window_count > SEARCH_WINDOW) { // window is complete
					// we have a new peak!
					last_peak_val = tentative_max_val;
					last_peak_idx = tentative_max_idx;
					last_peak_pfx = tentative_max_pfx;
					transmit_idx(last_peak_idx, last_peak_pfx, last_peak_val);
					
					//reset for next peak:
					tentative_max_val = 0;
					tentative_max_idx = 0;
					tentative_max_pfx = 0;

					//update search_threshold
					search_threshold = last_peak_val - (last_peak_val/SEARCH_THRESHOLD_REDUCTION);

					//move to DEAD_ZONE
					window_count = 0;
					search_sub_state = SEARCH_STATE_DEAD_ZONE;
					
				} else if (cur_val > tentative_max_val) { //new max
					tentative_max_val = cur_val;
					tentative_max_idx = cur_idx;
					tentative_max_pfx = cur_pfx;
				}

				cur_idx++;
				break;

			case SEARCH_STATE_DEAD_ZONE: //------------------------------------------------------------------
				
				if (cur_idx + DEAD_ZONE >= BUF_LEN) { // if dead zone wraps around buffer end
					global_state = IDLE_STATE;
				}

				//leap over dead_zone:
				cur_idx = (cur_idx + DEAD_ZONE) % BUF_LEN;
				search_sub_state = SEARCH_STATE_FIND_SIGNAL_SPARSE;

				cur_idx++;
				break;
							
			
			default: //------------------------------------------------------------------
				break;
		} // switch

		

		// conditions to escape search mode
		if ((global_state == PROC_BUF_1ST_HLF_STATE) && (cur_idx >= (BUF_LEN/2))) {
			global_state = IDLE_STATE;
			break;
		} else if (global_state == PROC_BUF_2ND_HLF_STATE && (cur_idx >= (BUF_LEN))) {
			global_state = IDLE_STATE;
			cur_idx = 0;
			break;
		} else if (global_state == ERROR_1_STATE) {
			break;
		} else if (global_state == IDLE_STATE) { // if dead zone wraps around buffer end
			break;
		}

	} //while

}








#endif
