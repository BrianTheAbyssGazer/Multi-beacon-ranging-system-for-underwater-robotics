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
    //ping_out.start_periodic_scheduler(5);
    //PingOut::debug = true;

    /************* Other transponder parameters **********/
    // int response_delay = 1; //units of full buffer temporal length (one less, so 0 is actually 1)
    int last_pfx = 0;
    uint8_t id = 3; // change this for different beacons
    bool signal[15]={0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
    uint16_t waiting_time = 0;
    uint8_t dataPos[] = {3, 5, 6, 7, 9, 10, 11, 12};
    int global_pfx = 0;
    bool echo=true;
    bool reset_gain=false;
    while (1) {
    	Timestamp tmsp = max_peak_detector.detect_peak();
		global_pfx = ping_out.cur_out_pfx;
		if(global_pfx!=last_pfx){
			waiting_time++;
			if(reset_gain){
				pgas.setGain(8);
				reset_gain = false;
			}
		}
		last_pfx = global_pfx;

		if(tmsp.pfx==-1){
			if (waiting_time>14 && signal[0]==1){
				for(int i=0;i<15;i++) signal[i]=0;
			}
		}
		else if(echo){
			echo=false;
			waiting_time = 0;
			reset_gain = true;
		}
		else{
			if(signal[0]==0) waiting_time = 0;
			signal[waiting_time] = 1;
			//max_peak_detector.send_data2computer(global_pfx,tmsp.pfx,waiting_time);
			if(waiting_time==14){
				uint8_t data = 0;
				for (int i = 0; i < 8; ++i) {
					data |= (signal[dataPos[i]] ? 1 : 0) << (7 - i);
				}
				if(data==id){
					ping_out.schedule_ping(tmsp.idx,(tmsp.pfx+1)%(0x8000));
					waiting_time = 0;
					pgas.setGain(2);
				}
				echo=true;
				for(int i=0;i<15;i++) signal[i]=0;
			}
		}
		ping_out.update();
    }
}

#endif

