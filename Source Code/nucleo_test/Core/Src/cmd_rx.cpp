/*
 * cmd_rx.cpp
 *
 *  Created on: Sep 3, 2024
 *      Author: arthur
 */

#include "main.h"
#include "cmd_rx.h"
#include "global_buffer_def.h"

//static initialization:
volatile int CMD_RX::last_cmd_type = CMD_NOT_RX;
volatile bool CMD_RX::rx_cplt = false;
volatile int CMD_RX::last_cmd_detail = 0;
volatile uint8_t CMD_RX::last_cmd_data[8] = {0,0,0,0, 0,0,0,0};
uint8_t CMD_RX::rx_buf[CRX_PACKET_LEN] = {CMD_NOT_RX, 0};
uint8_t CMD_RX::unit_test_buf[8] = {0,0,0,0, 0,0,0,0};


CMD_RX::CMD_RX(UART_HandleTypeDef* p_huart) {
    this->p_huart = p_huart;

}

void CMD_RX::start_receive(void){
    last_cmd_type = CMD_NOT_RX;
    last_cmd_detail = 0;
    HAL_UART_Receive_IT(p_huart, rx_buf, CRX_PACKET_LEN);
}
void CMD_RX::start_unit_test(void){
	rx_cplt = false;
    //static const uint8_t flag[5]={0xFF,0,0,0,9};
    //HAL_UART_Transmit_IT(p_huart, flag, 5);
    HAL_UART_Receive_IT(p_huart, unit_test_buf, DATA_LEN);
}

int CMD_RX::get_cmd_type(void) {
    return last_cmd_type;
}


int CMD_RX::get_cmd_detail(void) {
    return last_cmd_detail;
}

uint8_t* CMD_RX::get_cmd_data(void) {
    return (uint8_t*)last_cmd_data;
}


extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	CMD_RX::last_cmd_type   = CMD_RX::rx_buf[0];
    CMD_RX::last_cmd_detail = CMD_RX::rx_buf[1];
    CMD_RX::rx_cplt = true;
    for(int i=0;i<DATA_LEN;i++)CMD_RX::last_cmd_data[i] = CMD_RX::unit_test_buf[i];
}


