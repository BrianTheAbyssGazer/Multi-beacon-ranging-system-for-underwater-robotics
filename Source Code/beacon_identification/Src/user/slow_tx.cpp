/*
 * slow_tx.cpp
 *
 *  Created on: Sep 18, 2024
 *      Author: arthur
 */

#include "mode.h"
#include "main.h"
#include "cmd_rx.h"
#include "ping_out.h"
#include "slow_tx.h"

#if SLOW_TX_MODE



extern "C" void slow_tx_main(
                                UART_HandleTypeDef* p_huart,
                                DMA_HandleTypeDef* p_hdma_tim2_up,
                                TIM_HandleTypeDef* p_htim2) {

    /******************* SETUP CMD RX ************************/
    //CMD_RX cmd_rx(p_huart);
	//cmd_rx.start_receive();

    /******************* SETUP TX ************************/
    PingOut ping_out(p_hdma_tim2_up, p_htim2);
    ping_out.start_periodic_scheduler(10);


    while (1) {

        ping_out.update();

        //TODO we can add command RX check here

    }
}



#endif




