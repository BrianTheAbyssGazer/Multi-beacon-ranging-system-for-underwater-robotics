/*
 * tof_master.cpp
 *
 *  Created on: Sep 18, 2024
 *      Author: arthur
 */

#include "mode.h"
#include "main.h"
#include "max_peak_detector.h"
#include "pga.h"
#include "cmd_rx.h"
#include "ping_out.h"


#if TIME_OF_FLIGHT_MODE





extern "C" void tof_master_main(ADC_HandleTypeDef* p_hadc,
                                TIM_HandleTypeDef* p_htim3,
                                UART_HandleTypeDef* p_huart,
                                OPAMP_HandleTypeDef* p_opamp_1,
                                OPAMP_HandleTypeDef* p_opamp_2,
                                DMA_HandleTypeDef* p_hdma_tim2_up,
                                TIM_HandleTypeDef* p_htim2) {

    /******************* SETUP RX ************************/
    //CMD_RX cmd_rx(p_huart);
	IndexInfoTX idx_info_tx(p_huart);
	PGA_cascade_2 pgas(p_opamp_1, p_opamp_2);
    pgas.setGain(8);


	//cmd_rx.start_receive();

	MaxPeakDetector max_peak_detector(p_hadc, p_htim3,  &idx_info_tx);

	//parameter tweaking:
	max_peak_detector.min_aid = true;
    //max_peak_detector.search_threshold_reduction = 5;
    //max_peak_detector.search_window = 200;
    //max_peak_detector.dead_zone_len = 4000;
    //max_peak_detector.search_threshold = 2500;




    /******************* SETUP TX ************************/
    PingOut ping_out(p_hdma_tim2_up, p_htim2);
    ping_out.start_periodic_scheduler(20);
    //PingOut::debug = true;

    Timestamp first_peak_tmsp(-1, -1);
    Timestamp cur_peak_tmsp(-1,-1);
    uint16_t consecutive_pulses_detected = 0;
    while (1) {
    	cur_peak_tmsp = max_peak_detector.detect_peak();
    	if (cur_peak_tmsp.pfx!=-1){
    		if(first_peak_tmsp.pfx==-1){
    			first_peak_tmsp = cur_peak_tmsp;
    			consecutive_pulses_detected = 1;
    		}
    		else if(cur_peak_tmsp.pfx==(first_peak_tmsp.pfx + 1)%(0x8000) ||
    				cur_peak_tmsp.pfx==(first_peak_tmsp.pfx + 2)%(0x8000)){
    			consecutive_pulses_detected++;
    		}
    		else {
    			first_peak_tmsp = cur_peak_tmsp;
    			consecutive_pulses_detected = 1;
    		}
    	}
    	else if(first_peak_tmsp.pfx != -1 &&
    			cur_peak_tmsp.pfx!=first_peak_tmsp.pfx &&
				cur_peak_tmsp.pfx!= (first_peak_tmsp.pfx + 1)%(0x8000) &&
				cur_peak_tmsp.pfx!= (first_peak_tmsp.pfx + 2)%(0x8000){
    		max_peak_detector.send_data2computer(first_peak_tmsp, consecutive_pulses_detected);
    		first_peak_tmsp.pfx = -1;
    		consecutive_pulses_detected = 0;
    	}

    	ping_out.update();
    }
}

#endif


