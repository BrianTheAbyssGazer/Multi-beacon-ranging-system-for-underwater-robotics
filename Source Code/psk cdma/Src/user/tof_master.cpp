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
    pgas.setGain(gain);

    /******************* SETUP TX ************************/
	IndexInfoTX idx_info_tx(p_huart);
    PingOut ping_out(p_hdma_tim2_up, p_htim2, &idx_info_tx);
    //ping_out.start_periodic_scheduler(300);
    PingOut::debug = true;
	MaxPeakDetector max_peak_detector(p_hadc, p_htim3, &idx_info_tx);
	max_peak_detector.min_aid = true;
	//uint16_t pre_pfx=0;
	int inc=1;
    while (1) {
		max_peak_detector.detect_peak();
		//pre_pfx=max_peak_detector.cur_pfx;

		if(ping_out.cur_out_pfx+1-ping_out.scheduled_pfx>1800){//to be wrapped around
			ping_out.schedule(ping_out.cur_out_pfx+1,(ping_out.scheduled_idx)%OUT_BUF_LEN);
			max_peak_detector.signal_flag=true;
		}
		if(ping_out.scheduled_flag){
			//if(gain>=3)inc=-1;
			//else if(gain<=2)inc=1;
			//gain+=inc;
			//pgas.setGain(gain);
			ping_out.scheduled_flag=false;
		}
		//idx_info_tx.stream_adc(gain[1]);
		ping_out.update();
	}
}
#endif
