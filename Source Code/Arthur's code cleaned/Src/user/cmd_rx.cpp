/*
 * cmd_rx.cpp
 *
 *  Created on: Sep 3, 2024
 *      Author: arthur
 */

#include "main.h"
#include "cmd_rx.h"

//static initialization:
volatile int CMD_RX::last_cmd_type = CMD_NOT_RX;
volatile int CMD_RX::last_cmd_detail = 0;
uint8_t CMD_RX::rx_buf[CRX_PACKET_LEN] = {CMD_NOT_RX, 0};


CMD_RX::CMD_RX(UART_HandleTypeDef* p_huart) {
    this->p_huart = p_huart;

}




void CMD_RX::start_receive(){
    last_cmd_type = CMD_NOT_RX;
    last_cmd_detail = 0;
    HAL_UART_Receive_IT(p_huart, rx_buf, CRX_PACKET_LEN);
}


int CMD_RX::get_cmd_type(void) {
    return last_cmd_type;
}


int CMD_RX::get_cmd_detail(void) {
    return last_cmd_detail;
}




extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	CMD_RX::last_cmd_type   = CMD_RX::rx_buf[0];
    CMD_RX::last_cmd_detail = CMD_RX::rx_buf[1];
}


