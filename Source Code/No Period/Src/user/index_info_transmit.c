/*
 * index_info_transmit.c
 *
 *  Created on: Aug 22, 2024
 *      Author: arthur
 */


/*
 * Very simple small packet structure for UART.
 * Used to communicate indexes of peaks, etc. and error or other simple messages.
 */

#include "main.h"
#include "index_info_transmit.h"




/****************** PACKET HEADERS ******************/
#define INDEX_HEADER  0xF0 //6 byte payload with buf_idx, pre_idx, peak_val
#define INFO_1_HEADER 0xF1 //reserved
#define INFO_2_HEADER 0xF2 //reserved
#define INFO_3_HEADER 0xF3 //reserved
#define ERR_1_HEADER  0xF8 //indicates next buffer is full before we have finished processing the previous
#define ERR_2_HEADER  0xF9 //reserved
#define ERR_3_HEADER  0xFA //reserved
/*************************************************** */


/*********GLOBALS*************/
UART_HandleTypeDef* huart_pointer;
#define PACKET_LEN 7 //bytes

uint8_t send_buf[PACKET_LEN];
/*************************/




//Call to set the huart handle that will be used for all other calls.
//This function must be called before any other transmit function.
void index_info_transmit_init(UART_HandleTypeDef* p_huart){
    huart_pointer = p_huart;
}


// Send an index packet, which contains peak index, index prefix and peak value.
// LSBys sent first.
/*
Args:
 - buffer index
 - index prefix
 - peak value (at index)
*/
void transmit_idx(int buf_idx, int pre_idx, uint16_t peak_val) {
    send_buf[0] = INDEX_HEADER;
    *((uint16_t*)(send_buf + 1)) = (uint16_t) buf_idx;
    *((uint16_t*)(send_buf + 3)) = (uint16_t) pre_idx ;
    *((uint16_t*)(send_buf + 5)) = peak_val;
	HAL_UART_Transmit_IT(huart_pointer, send_buf, PACKET_LEN); 
}


//Send error1 packet
/*
Args:
 - buffer index
 - index prefix
*/
void transmit_err_1(int buf_idx, int pre_idx) {
    send_buf[0] = ERR_1_HEADER;
    *((uint16_t*)(send_buf + 1)) = (uint16_t) buf_idx;
    *((uint16_t*)(send_buf + 3)) = (uint16_t) pre_idx;
    send_buf[5] = 0;
    send_buf[6] = 0;
	HAL_UART_Transmit_IT(huart_pointer, send_buf, PACKET_LEN);
}



// For debugging
void test_pattern(UART_HandleTypeDef* p_huart) {
    index_info_transmit_init(p_huart);
    int i=0;
    int j=0;
    uint16_t k = 0;
    while (1) {
        transmit_idx(i, j, k);
        HAL_Delay(100);
        //transmit_err_1(0,0);
        //HAL_Delay(400);
        i+=1;
        j+=2;
        k+=3;
        if (i >= 24000) i = 0;
        if (j >= (1<<15)) j = 0;
        if (k >= (1<<12)) k = 0;
    }
}




