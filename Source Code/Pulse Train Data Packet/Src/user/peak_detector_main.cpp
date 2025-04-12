/*
 * peak_detector_main.cpp
 *
 *  Created on: Sep 11, 2024
 *      Author: arthur
 */

#include "mode.h"
#include "main.h"
#include "peak_detector_main.h"
#include "max_peak_detector.h"
#include "pga.h"
#include "cmd_rx.h"

#if BASIC_PEAK_DETECTOR_MODE


extern "C" void peak_detector_main(ADC_HandleTypeDef* p_hadc, TIM_HandleTypeDef* p_htim, UART_HandleTypeDef* p_huart, OPAMP_HandleTypeDef* p_opamp_1, OPAMP_HandleTypeDef* p_opamp_2) {
	
	CMD_RX cmd_rx(p_huart);
	IndexInfoTX idx_info_tx(p_huart);
	PGA_cascade_2 pgas(p_opamp_1, p_opamp_2);

	
	cmd_rx.start_receive();

	MaxPeakDetector max_peak_detector(p_hadc, p_htim,  &idx_info_tx);

	//parameter tweaking:
	max_peak_detector.min_aid = true;
    //max_peak_detector.search_threshold_reduction = 5; 
    //max_peak_detector.search_window = 200;
    //max_peak_detector.dead_zone_len = 4000;
    //max_peak_detector.search_threshold = 2500;


	while (1) {
		//idx_info_tx.test_pattern(); // debugging
		/********************* PEAK SEARCHING ********************/
		max_peak_detector.detect_peak();


		/********************* HANDLE COMMANDS ********************/
		if (cmd_rx.get_cmd_type() != CMD_NOT_RX) {
			// we got a command

			switch (cmd_rx.get_cmd_type())
			{
			case CMD_SET_GAIN_PGA_2:
				pgas.pga1.setGain((int) cmd_rx.get_cmd_detail());
				break;

			case CMD_SET_GAIN_PGA_3:
				pgas.pga2.setGain((int) cmd_rx.get_cmd_detail());
				break;

			case CMD_SET_GAIN_CASCADE:
				pgas.setGain((int) cmd_rx.get_cmd_detail());
				break;

			default:
				break;
			}

			cmd_rx.start_receive();

		} //if (cmd_rx.get_cmd_type() != CMD_NOT_RX)


		


	}
	
	return;
}

#endif
