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




extern "C" void tof_master_main(ADC_HandleTypeDef* p_hadc,
                                TIM_HandleTypeDef* p_htim3,
                                UART_HandleTypeDef* p_huart,
                                DMA_HandleTypeDef* p_hdma_tim2_up,
                                TIM_HandleTypeDef* p_htim2) {

    /******************* SETUP RX ************************/
    CMD_RX cmd_rx(p_huart);
    cmd_rx.start_unit_test();

	MaxPeakDetector max_peak_detector(p_hadc, p_htim3,  &idx_info_tx);
	max_peak_detector.min_aid = true;

    /******************* SETUP TX ************************/
	IndexInfoTX idx_info_tx(p_huart);
    PingOut ping_out(p_hdma_tim2_up, p_htim2);
    ping_out.start_periodic_scheduler(50);
    //PingOut::debug = true;

    while (1) {
		if (cmd_rx.get_cmd_type() == -1) {
			ping_out.set_phase_keying_data(cmd_rx.get_cmd_detail());
			cmd_rx.start_unit_test();
		}
		ping_out.update();
    }
}

