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
    pgas.setGain(2);

    /******************* SETUP TX ************************/
	IndexInfoTX idx_info_tx(p_huart);
    PingOut ping_out(p_hdma_tim2_up, p_htim2, &idx_info_tx);
    //ping_out.start_periodic_scheduler(50);
    //PingOut::debug = true;
	MaxPeakDetector max_peak_detector(p_hadc, p_htim3,  &idx_info_tx);

    while (1) {
    	if(max_peak_detector.global_state == ping_out.po_state){
        	max_peak_detector.detect_peak();
        	if(max_peak_detector.signal_flag){
        		uint16_t idx_peak=max_peak_detector.last_peak_idx;
        		uint16_t pfx_peak=max_peak_detector.last_peak_pfx;
    			if (idx_peak<HAL_BUF_LEN){
    				idx_peak+=HAL_BUF_LEN;
    			}
    			else {
    				idx_peak-=HAL_BUF_LEN;
    				pfx_peak++;
    			}
    			idx_peak/=6;
    			pfx_peak+=DATA_PFX;
    			max_peak_detector.mark_pinout(pfx_peak,idx_peak*6);
    			ping_out.schedule(pfx_peak,idx_peak);
    			max_peak_detector.signal_flag=false;
        	}
        	ping_out.update();
    	}
    }
}
#endif
