/*
 * transponder.c
 *
 *  Created on: Sep 6, 2024
 *      Author: arthur
 */


#include "mode.h"
#include "main.h"
#include "max_peak_detector.h"
#include "pga.h"
#include "cmd_rx.h"
#include "ping_out.h"


#if TRANSPONDER_MODE

uint8_t gain=8;

extern "C" void transponder_main(ADC_HandleTypeDef* p_hadc, 
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
	pgas.setGain(gain);
	
	//cmd_rx.start_receive();

	MaxPeakDetector max_peak_detector(p_hadc, p_htim3,  &idx_info_tx);

    /******************* SETUP TX ************************/
    PingOut ping_out(p_hdma_tim2_up, p_htim2, &idx_info_tx);
    //ping_out.start_periodic_scheduler(40);
    //PingOut::debug = true;

    while (1) {
    	if(max_peak_detector.global_state == ping_out.po_state){
        	max_peak_detector.detect_peak();
        	if(max_peak_detector.signal_flag) {
        		ping_out.enable_scheduler = false;
        	}
        	if(max_peak_detector.data_flag){
    			ping_out.schedule(max_peak_detector.pinout_pfx,max_peak_detector.pinout_idx/6);
    			max_peak_detector.data_flag=false;
        		max_peak_detector.signal_flag =false;
        	}
        	if(ping_out.scheduled_flag){
        	    pgas.setGain(gain);
        		ping_out.scheduled_flag=false;
        	}
        	ping_out.update();
    	}
    }
}
#endif

