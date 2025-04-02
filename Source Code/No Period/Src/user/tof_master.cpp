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
    pgas.setGain(2);


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
    int response_delay = 1; //units of full buffer temporal length (one less, so 0 is actually 1)
    int cooldown_duration = 10; //units of full buffer temporal length
    int resend_waiting_duration = 50; //if no signal is detected in 50 buffers resend request to transponder.
    int last_sent_pfx = 0;

    while (1) {

    	Timestamp pt = max_peak_detector.detect_peak();
    	if (pt.pfx != -1 && pt.pfx > last_sent_pfx + cooldown_duration) {
    		ping_out.schedule_ping(pt.idx, pt.pfx + response_delay);
    	    last_sent_pfx = pt.pfx + response_delay;
    	}
    	else if (pt.pfx == -1 && pt.pfx > last_sent_pfx + resend_waiting_duration){
    		ping_out.schedule_ping(pt.idx, pt.pfx + response_delay);
    	    last_sent_pfx = pt.pfx + response_delay;
    	}
        ping_out.update();

        //TODO we can add command RX check here

    }
}

#endif


