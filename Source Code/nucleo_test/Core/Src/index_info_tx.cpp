/*
 * index_info_rx.cpp
 *
 *  Created on: Sep 11, 2024
 *      Author: arthur
 */


#include "main.h"
#include "index_info_tx.h"

IndexInfoTX :: IndexInfoTX(UART_HandleTypeDef* p_huart) {
    this->p_huart = p_huart;
}


// Send an index packet, which contains peak index, index prefix and peak value.
// LSBys sent first.
/*
Args:
 - buffer index
 - index prefix
 - peak value (at index)
*/
void IndexInfoTX :: transmit_idx(int buf_idx, int pre_idx, uint16_t peak_val) {
    send_buf[0] = IdxInfoHeader::INDEX;
    *((uint16_t*)(send_buf + 1)) = (uint16_t) buf_idx;
    *((uint16_t*)(send_buf + 3)) = (uint16_t) pre_idx;
    *((uint16_t*)(send_buf + 5)) = peak_val;
	HAL_UART_Transmit_IT(p_huart, send_buf, IITX_PACKET_LEN); 
}

void IndexInfoTX :: stream_adc(uint8_t adc_val) {
	static uint8_t bytes[5]={0xF0,0,0,0,7};
	bytes[1] = static_cast<uint8_t>(adc_val >> 0);
	bytes[2] = static_cast<uint8_t>(adc_val >> 8);
	bytes[3] = static_cast<uint8_t>(adc_val >> 16);
	bytes[4] = static_cast<uint8_t>(adc_val >> 24);
	HAL_UART_Transmit_IT(p_huart, bytes, 5);
}
void IndexInfoTX :: send_flag(int flag_val) {
	static uint8_t bytes[5]={0xFF,0,0,0,0};
	bytes[1] = static_cast<uint8_t>(flag_val >> 0);
	bytes[2] = static_cast<uint8_t>(flag_val >> 8);
	bytes[3] = static_cast<uint8_t>(flag_val >> 16);
	bytes[4] = static_cast<uint8_t>(flag_val >> 24);
	HAL_UART_Transmit_IT(p_huart, bytes, 5);
}

//Send error1 packet
/*
Args:
 - buffer index
 - index prefix
*/
void IndexInfoTX :: transmit_err_1(int buf_idx, int pre_idx) {
    send_buf[0] = IdxInfoHeader::ERR_1;
    *((uint16_t*)(send_buf + 1)) = (uint16_t) buf_idx;
    *((uint16_t*)(send_buf + 3)) = (uint16_t) pre_idx;
    send_buf[5] = 0;
    send_buf[6] = 0;
	HAL_UART_Transmit_IT(p_huart, send_buf, IITX_PACKET_LEN);
}



// For debugging
void IndexInfoTX :: test_pattern() {
    int i=0;
    int j=0;
    uint16_t k = 0;
    while (1) {
        transmit_idx(i, j, k);
        HAL_Delay(100);
        transmit_err_1(0,0);
        HAL_Delay(400);
        i+=1;
        j+=2;
        k+=3;
        if (i >= 24000) i = 0;
        if (j >= (1<<15)) j = 0;
        if (k >= (1<<12)) k = 0;
    }
}
