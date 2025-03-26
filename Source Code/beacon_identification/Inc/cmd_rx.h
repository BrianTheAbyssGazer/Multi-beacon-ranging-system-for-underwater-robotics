/*
 * cmd_rx.h
 *
 *  Created on: Sep 3, 2024
 *      Author: arthur
 */



/* packet structure */
#define CRX_PACKET_LEN 2
// byte 0         | byte 1
// COMMAND TYPE   |   COMMAND DETAIL

// Gain setting commands enum. Detail byte is the index of the gain to set */
#define CMD_SET_GAIN_PGA_2 0xF1
#define CMD_SET_GAIN_PGA_3 0xF2
#define CMD_SET_GAIN_CASCADE 0xF3

// The "no command yet recieved" type
#define CMD_NOT_RX 0x00



#ifdef __cplusplus

class CMD_RX
{
    private:
        UART_HandleTypeDef* p_huart;
        
    public:
        static uint8_t rx_buf[CRX_PACKET_LEN];
        static volatile int last_cmd_type;
        static volatile int last_cmd_detail;
        CMD_RX(UART_HandleTypeDef* p_huart);
        void start_receive(void);
        int get_cmd_type(void);
        int get_cmd_detail(void);
        
};


extern "C" {

#endif

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif
