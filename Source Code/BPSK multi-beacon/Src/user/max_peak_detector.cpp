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
#include "arm_math.h"

#if BASIC_PEAK_DETECTOR_MODE || TRANSPONDER_MODE || TIME_OF_FLIGHT_MODE || ECHO_MASTER_MODE



// global ADC buffer:
#if TRANSPONDER_MODE
static constexpr uint8_t my_id[STRING_LEN] = {
    static_cast<uint8_t>(id_list[ID*2]),
    static_cast<uint8_t>(id_list[ID*2+1]),
};
#endif
uint16_t buf[BUF_LEN];
#if STREAM
uint16_t uart_buf[UART_BUF_LEN];
uint16_t ccm_capture_buffer[CCM_BUF_LEN] __attribute__((section(".ccmram")));
#elif DECODE||DEBUG_TIM
uint16_t buf_res[3]={1884,1884,1884};
#endif

//initialise statics:
volatile uint16_t MaxPeakDetector::global_state = MPDState::PROC_BUF_2ND_HLF;
volatile uint16_t MaxPeakDetector::cur_pfx = 0; // incremented each time the ADC buffer completely fills
/* Initialisation:
    // parameters
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
    dead_zone_count = 0;
#if TIME_OF_FLIGHT_MODE
    beacon_id=0;
#endif
#if DECODE
    phase = 0;
    phase_int=0;
    sample_counter=0;
    symbol_counter=0;
    corr_sum=0;
    signal_flag = false;
    data_flag = false;
	pinout_pfx = INIT_OUT;
	pinout_idx = HAL_BUF_LEN;
    enable_pfx = DATA_PFX+1;
    enable_idx = 0;
	last_peak_pfx=TIMEOUT;
	last_peak_idx=HAL_BUF_LEN+12;
	last_peak_val=1884;
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
void MaxPeakDetector :: detect_peak() {
    //error_1_handle();
    //HAL_Delay(1000);
    switch (global_state)
    {
        case MPDState::PROC_BUF_1ST_HLF:
            if (cur_idx < HAL_BUF_LEN) {
                search_loop(0);
            } else {
                global_state = MPDState::IDLE;
            }
            break;

        case MPDState::PROC_BUF_2ND_HLF:
            if (cur_idx >= HAL_BUF_LEN) {
                search_loop(HAL_BUF_LEN);
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
}

void MaxPeakDetector :: search_loop(uint16_t offset) {
	uint16_t cur_val;
	while (1) {
		cur_val = buf[cur_idx];
		float sin,cos,real,imag,i_channel,q_channel; // Masking instead of %

#if STREAM
		switch (search_sub_state){
			case MPDSearchState::NO_SIGNAL: //------------------------------------------------------------------
				if (cur_val > 2384||cur_val<1384) {
					uart_idx=0;
					ccm_idx=0;
					search_sub_state = MPDSearchState::YES_SIGNAL;
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
#else
		switch (search_sub_state){
			case MPDSearchState::NO_SIGNAL: //------------------------------------------------------------------
				if (cur_val > 2184||cur_val<1484) {
				    sample_counter=0;
				    symbol_counter=0;
					search_sub_state = MPDSearchState::YES_SIGNAL;
					signal_flag=true;
				}
				else cur_idx+=2;
				break;
#if DECODE
			case MPDSearchState::YES_SIGNAL: //------------------------------------------------------------------
				if(symbol_counter<DATA_LEN){
					if(sample_counter<6*N_CYCLE){
						uint16_t pre_val;
						if (cur_idx<offset+3) pre_val=buf_res[cur_idx-offset];
						else pre_val=buf[cur_idx-3];

						imag = (float(pre_val)-1884) * 0.0005f;// 1/2048=0.00048828125
						real = (float(cur_val) - 1884) * 0.0005f;
						arm_sin_cos_f32(phase* 57.2958f, &sin, &cos);
						i_channel=real*cos+imag*sin;
						q_channel=imag*cos-real*sin;
						phase += 0.523599 + (0.05 * i_channel * q_channel); // 1/12=0.5236 (12 samples per cycle), 0.05 is empirical
						if (phase>6.28319) phase-=6.28319;
						corr_sum+=i_channel;
						if(!symbol_counter){
							if(cur_val>last_peak_val){
								last_peak_val=cur_val;
								last_peak_idx=cur_idx;
								last_peak_pfx=cur_pfx;
							}
						}
						cur_idx++;
						sample_counter++;
					}
					else{
						uint8_t i_char = symbol_counter / 8; // to be changed to >>3
						uint8_t i_bit = symbol_counter & 7; // Find the bit position (0-7)
						if(corr_sum>0.0f){
							rx_data[i_char] |= (1 << i_bit);
						}
						else{
							rx_data[i_char] &= ~(1 << i_bit);
						}

						cur_idx+=(DEAD_INTERVAL-N_CYCLE)*6; //12 samples per cycle * 16 cycle per symbol * 7
						sample_counter=0;
						symbol_counter++;
						corr_sum=0;
					}
				}
				else{
					symbol_counter=0;
					sample_counter=0;
					corr_sum=0;
					phase=0;
					if (rx_data[0]&1) inverse_data=0;
					else inverse_data=0xFF;
					for (uint8_t j = 0; j < STRING_LEN; j++){
						rx_data[j]=rx_data[j]^inverse_data;
					}

					if(last_peak_pfx>pinout_pfx) delta_idx=uint32_t(last_peak_pfx-pinout_pfx)*BUF_LEN+last_peak_idx-pinout_idx;
					else delta_idx=uint32_t(0xFFFF-pinout_pfx+last_peak_pfx+1)*BUF_LEN+last_peak_idx-pinout_idx;
#if TRANSPONDER_MODE
					if(memcmp(rx_data, my_id, STRING_LEN) == 0) {
						data_flag=true;
						if (last_peak_val>3567 && gain>2){
							gain--;
						}
						else if(last_peak_val<2361 && gain<8){
							gain++;
						}
					}
					//(*p_index_info_tx).stream_adc(uint16_t(gain));

					//for (uint8_t j = 0; j < 4; j++) rx_data[STRING_LEN+j] = uint8_t((delta_idx>>(j*8)) & 0xFF);
					for (uint8_t j = 0; j < 2; j++) rx_data[STRING_LEN+j] = uint8_t((last_peak_val>>(j*8)) & 0xFF);
					rx_data[STRING_LEN+2] = gain; // 0x78
					rx_data[STRING_LEN+3] = 0; // 0x78
					(*p_index_info_tx).send_bytes(rx_data);

#elif TIME_OF_FLIGHT_MODE
					data_flag=true;
					//for (uint8_t j = 0; j < 4; j++) rx_data[STRING_LEN+j] = uint8_t((delta_idx>>(j*8)) & 0xFF); // 0x78
					//(*p_index_info_tx).send_bytes(rx_data);
					if(memcmp(rx_data,&id_list[beacon_id*2], STRING_LEN) == 0){
						if (last_peak_val>3567 && gain[beacon_id]>2){
							gain[beacon_id]--;
						}
						else if(last_peak_val<2361 && gain[beacon_id]<8){
							gain[beacon_id]++;
						}
					}
					//else if(gain[beacon_id]>2){
					//	gain[beacon_id]--;
					//}
					//(*p_index_info_tx).stream_adc(uint16_t(gain[beacon_id]));

#endif
					//timing of pinout
					pinout_pfx = last_peak_pfx+DATA_PFX;
					pinout_idx = (last_peak_idx/6)*6;
					mark_pinout();
					search_sub_state = MPDSearchState::DEMODULATOR_DISABLED;
					last_peak_val=1884;
				}
				break;
#elif DEBUG_TIM
			case MPDSearchState::YES_SIGNAL: //------------------------------------------------------------------
				if(symbol_counter<DATA_LEN){
					if(sample_counter<6*N_CYCLE){
						corr_sum+=cur_val/100;
						cur_idx++;
						sample_counter++;
						if(!symbol_counter){
							if(cur_val>last_peak_val){
								last_peak_val=cur_val;
								last_peak_idx=cur_idx;
								last_peak_pfx=cur_pfx;
							}
						}
					}
					else{
						cur_idx+=(DEAD_INTERVAL-N_CYCLE)*6; //12 samples per cycle * 16 cycle per symbol * 7
						sample_counter=0;
						symbol_counter++;
						(*p_index_info_tx).stream_adc(corr_sum);
						corr_sum=0;
					}
				}
				else{
					symbol_counter=0;
					sample_counter=0;
					corr_sum=0;

					enable_pfx=cur_pfx+1;
					enable_idx=cur_idx;
					pinout_pfx = last_peak_pfx+DATA_PFX;
					pinout_idx = (last_peak_idx/6)*6;
					mark_pinout();
					search_sub_state = MPDSearchState::DEMODULATOR_DISABLED;
					last_peak_val=1884;
				}
				break;
#endif
			case MPDSearchState::DEMODULATOR_DISABLED:
				if(cur_pfx<enable_pfx)
					cur_idx+=HAL_BUF_LEN;
				else if(cur_pfx==enable_pfx){
					if(cur_idx>=enable_idx){
						search_sub_state = MPDSearchState::NO_SIGNAL;
						}
					else cur_idx+=2;
				}
				else {
					search_sub_state = MPDSearchState::NO_SIGNAL;
				}
				break;
		} // switch
#endif
		// conditions to escape search mode
		if ((global_state == MPDState::PROC_BUF_1ST_HLF) && (cur_idx >= (HAL_BUF_LEN))) {
			global_state = MPDState::IDLE;
#if !STREAM
			for (uint8_t i=0;i<3;i++)buf_res[i]=buf[HAL_BUF_LEN-3+i];
#endif
			break;
		} else if (global_state == MPDState::PROC_BUF_2ND_HLF && (cur_idx >= (BUF_LEN))) {
			global_state = MPDState::IDLE;
			cur_idx = cur_idx % BUF_LEN;
#if !STREAM
			for (uint8_t i=0;i<3;i++)buf_res[i]=buf[BUF_LEN-3+i];
#endif
			break;
		} else if (global_state == MPDState::ERROR_1) {
			break;
		}
	} //while
}
void MaxPeakDetector::mark_pinout() {
	//timing for recover receiving after pinout
	enable_pfx = pinout_pfx+DATA_PFX+1;  // delay from pingout to actual sound wave is 2pfx+1008idx
	enable_idx = pinout_idx+HAL_BUF_LEN; // delay from pingout to actual sound wave is 2pfx+1008idx
	if(enable_idx>=BUF_LEN){
		enable_idx-=BUF_LEN;
		enable_pfx++;
	}
}

void MaxPeakDetector :: error_1_handle() {
    //(*p_index_info_tx).stream_adc(7); //send error notification
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

			case MPDState::ERROR_1:
				MaxPeakDetector::global_state = MPDState::IDLE;
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
				
			case MPDState::ERROR_1:
				MaxPeakDetector::global_state = MPDState::IDLE;
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
