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
    //ping_out.start_periodic_scheduler(50);
    uint16_t max_waiting_time = 100;  //needs to be greater than signal length.
    uint16_t waiting_time = 0;
    uint8_t id=1;
    int echo_pfx = -1;
    uint16_t echo = 0;
    bool debug=false;
    int global_pfx = 0;
    int last_pfx = 0;
    //PingOut::debug = true;

    while (1) {
    	Timestamp tmsp = max_peak_detector.detect_peak();
		global_pfx = ping_out.cur_out_pfx;
		if(global_pfx!=last_pfx){
			waiting_time++;
		}
		last_pfx = global_pfx;
		if(tmsp.pfx==-1){
			if(waiting_time>=max_waiting_time){
				if (debug) ping_out.start_datapacket_scheduler(id);
				else echo = ping_out.start_datapacket_scheduler(id);
				echo_pfx = (global_pfx+15)%(0x8000);
				waiting_time = 0;
			}
			if (waiting_time>15 && echo!=0){
				echo = 0;
			}
		}
		else if(echo!=0) {
			echo--;
			if(echo==0)max_peak_detector.set_threshold(2100);
		}
		else{
			if(echo_pfx<tmsp.pfx){
				max_peak_detector.send_data2computer(tmsp.idx,tmsp.pfx-echo_pfx,uint16_t(id));
			}
			else{
				max_peak_detector.send_data2computer(tmsp.idx,(0x8000)-echo_pfx+tmsp.pfx,uint16_t(id));
			}
			if(id==3)id=1;
			else id++;
			waiting_time = 99;
		}
		ping_out.update();
    }
}

#endif


