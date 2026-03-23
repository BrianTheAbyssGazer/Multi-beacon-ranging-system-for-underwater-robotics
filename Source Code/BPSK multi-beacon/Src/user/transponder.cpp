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
	pgas.setGain(2);

	
	//cmd_rx.start_receive();

	MaxPeakDetector max_peak_detector(p_hadc, p_htim3,  &idx_info_tx);

    /******************* SETUP TX ************************/
    PingOut ping_out(p_hdma_tim2_up, p_htim2, &idx_info_tx);
    //ping_out.start_periodic_scheduler(40);
    //PingOut::debug = true;

    while (1) {
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
			pfx_peak+=(DATA_LEN/(OUT_BUF_LEN/DEAD_INTERVAL));
			max_peak_detector.mark_pinout(pfx_peak,idx_peak*6);
			ping_out.schedule(pfx_peak,idx_peak);
			max_peak_detector.signal_flag=false;
    	}
    	ping_out.update();
    }
}
#endif

