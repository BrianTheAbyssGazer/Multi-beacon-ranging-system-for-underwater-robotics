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
    PingOut ping_out(p_hdma_tim2_up, p_htim2);
    //ping_out.start_periodic_scheduler(500);
    //PingOut::debug = true;
	MaxPeakDetector max_peak_detector(p_hadc, p_htim3,  &idx_info_tx);
	max_peak_detector.min_aid = true;

    while (1) {
    	max_peak_detector.detect_peak();
    	//ping_out.update();
#if ECHO_MASTER_MODE
		if (cmd_rx.rx_cplt) {
			ping_out.set_phase_keying_data(cmd_rx.get_cmd_data());
			cmd_rx.start_unit_test();
		}
		ping_out.update();
#endif
    }
}
#endif
