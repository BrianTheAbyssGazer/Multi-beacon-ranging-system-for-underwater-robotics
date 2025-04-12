/*
 * adc_over_uart.cpp
 *
 *  Created on: Sep 3, 2024
 *      Author: arthur
 */


#include "main.h"
#include "adc_over_uart.h"
#include "global_buffer_def.h"
#include "mode.h"




#if ADC_OVER_UART_MODE


/************** ADC BUFFER etc. **************/
uint16_t buf[BUF_LEN];
volatile int ADC_over_UART::buffer_full_flag = 0;

/************** COMMAND RX *********************/
//TODO



// Initialisation
ADC_over_UART::ADC_over_UART(ADC_HandleTypeDef* p_hadc, TIM_HandleTypeDef* p_htim, UART_HandleTypeDef* p_huart, OPAMP_HandleTypeDef* p_opamp_1, OPAMP_HandleTypeDef* p_opamp_2)
: pgas(p_opamp_1, p_opamp_2), cmd_rx(p_huart) 
{
	this->p_hadc = p_hadc;
	this->p_huart = p_huart;

	//Initialize the start/stop signpost buffers
	for (int i=0; i<SIGNPOST_LEN; i++) {
		start_signpost[i] = START_SIGNAL;
		stop_signpost[i]  = STOP_SIGNAL;
	}


	HAL_TIM_Base_Start(p_htim);
	HAL_ADC_Start_DMA(p_hadc, (uint32_t*)buf, BUF_LEN);

}



void ADC_over_UART::main() {
	cmd_rx.start_receive();
	while (1)
	  {
		 

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
		}
		
		  if (buffer_full_flag == 1) {
			  //signal start of buffer
			  HAL_UART_Transmit(p_huart, start_signpost, SIGNPOST_LEN, 1000);

			  //send adc buffer over uart
			  HAL_UART_Transmit(p_huart, (uint8_t*)buf, (BUF_LEN)*2, 10000);

			  //signal end of buffer
			  HAL_UART_Transmit(p_huart, stop_signpost, SIGNPOST_LEN, 1000);

			  //restart DMA
			  HAL_ADC_Stop_DMA (p_hadc);
			  HAL_ADC_Start_DMA(p_hadc, (uint32_t*)buf, BUF_LEN);
			  buffer_full_flag = 0;
		  }
	  }
}








extern "C" void adc_over_uart(ADC_HandleTypeDef* p_hadc, TIM_HandleTypeDef* p_htim, UART_HandleTypeDef* p_huart, OPAMP_HandleTypeDef* p_opamp_1, OPAMP_HandleTypeDef* p_opamp_2) {
	ADC_over_UART fred(p_hadc, p_htim, p_huart, p_opamp_1, p_opamp_2);
	fred.main();
}



//Called when first half of buffer is filled
//extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
//	buffer_full_flag = 1;
//}

//Called when buffer is completely filled
extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	ADC_over_UART::buffer_full_flag = 1;
}


#endif



