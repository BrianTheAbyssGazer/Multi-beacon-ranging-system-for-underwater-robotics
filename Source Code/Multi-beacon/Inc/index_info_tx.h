/*
 * index_info_tx.h
 *
 *  Created on: Sep 11, 2024
 *      Author: arthur
 */

#ifdef __cplusplus

enum IdxInfoHeader {
     INDEX  = 0xF0, //6 byte payload with buf_idx, pre_idx, peak_val
     INFO_1 = 0xF1, //reserved
     INFO_2 = 0xF2, //reserved
     INFO_3 = 0xF3, //reserved
     ERR_1  = 0xF8, //indicates next buffer is full before we have finished processing the previous
     ERR_2  = 0xF9, //reserved
     ERR_3  = 0xFA, //reserved
};


#define IITX_PACKET_LEN 7 //bytes

class IndexInfoTX {
    public:
        UART_HandleTypeDef* p_huart;
        uint8_t send_buf[IITX_PACKET_LEN];

        IndexInfoTX(UART_HandleTypeDef*);
        void transmit_idx(int, int, uint16_t);
        void transmit_err_1(int, int);
        void test_pattern(void);
        
};

#endif