/*
 * tof_master.cpp
 *
 *  Created on: Sep 18, 2024
 *      Author: arthur
 */

#include "mode.h"
#include "main.h"
#include "max_peak_detector.h"
#include "cmd_rx.h"
#include "ping_out.h"
#include "pga.h"




#if ECHO_MASTER_MODE || TIME_OF_FLIGHT_MODE

uint8_t gain[3] = {2,2,2};
uint8_t lost_time[3] = {0,0,0};
extern "C" void tof_master_main(ADC_HandleTypeDef* p_hadc,
                                TIM_HandleTypeDef* p_htim3,
                                UART_HandleTypeDef* p_huart,
								OPAMP_HandleTypeDef* p_opamp_1,
								OPAMP_HandleTypeDef* p_opamp_2,
                                DMA_HandleTypeDef* p_hdma_tim2_up,
                                TIM_HandleTypeDef* p_htim2) {

    /******************* SETUP RX ************************/
    CMD_RX cmd_rx(p_huart);
#if ECHO_MASTER_MODE
    cmd_rx.start_unit_test();
#endif
	PGA_cascade_2 pgas(p_opamp_1, p_opamp_2);
    pgas.setGain(gain[0]);

    /******************* SETUP TX ************************/
	IndexInfoTX idx_info_tx(p_huart);
    PingOut ping_out(p_hdma_tim2_up, p_htim2, &idx_info_tx);
    //ping_out.start_periodic_scheduler(50);
    //PingOut::debug = true;
	MaxPeakDetector max_peak_detector(p_hadc, p_htim3,  &idx_info_tx);
    uint16_t prev_pfx=0;
    uint8_t time_since_last_pinout=0;
    while (1) {
    	if(max_peak_detector.global_state == ping_out.po_state){
        	max_peak_detector.detect_peak();
        	if(prev_pfx!=ping_out.cur_out_pfx){
        		time_since_last_pinout++;
        	}
        	if(max_peak_detector.signal_flag) {
        		ping_out.enable_scheduler = false;
        		time_since_last_pinout=0;
        	}
        	else if(time_since_last_pinout>TIMEOUT){//to be wrapped around
        		time_since_last_pinout=0;
    			ping_out.schedule(ping_out.cur_out_pfx+1,(ping_out.scheduled_idx+11)%OUT_BUF_LEN);
    			max_peak_detector.pinout_pfx = max_peak_detector.cur_pfx+1;
    			max_peak_detector.pinout_idx = (max_peak_detector.pinout_idx+66)%BUF_LEN;
        		//idx_info_tx.stream_adc(max_peak_detector.cur_pfx);
    			max_peak_detector.search_sub_state = MPDSearchState::DEMODULATOR_DISABLED;
        	}
        	if(max_peak_detector.data_flag){
        		time_since_last_pinout=0;
    			ping_out.schedule(max_peak_detector.pinout_pfx,max_peak_detector.pinout_idx/6);
    			max_peak_detector.data_flag=false;
        		max_peak_detector.signal_flag =false;
        	}
        	if(ping_out.scheduled_flag){
        		if(lost_time[ping_out.beacon_id]>3){
        			lost_time[ping_out.beacon_id]=0;
					if(gain[ping_out.beacon_id]<8)gain[ping_out.beacon_id]++;
        		}
        	    pgas.setGain(gain[ping_out.beacon_id]);
        	    max_peak_detector.beacon_id=ping_out.beacon_id;
				max_peak_detector.mark_pinout();
				lost_time[ping_out.beacon_id]++;
        		ping_out.scheduled_flag=false;
        	}
        	ping_out.update();
        	prev_pfx=ping_out.cur_out_pfx;
    	}
    }
}
#endif
