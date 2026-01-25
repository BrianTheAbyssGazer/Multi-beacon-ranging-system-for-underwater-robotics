/*
 * slow_tx.h
 *
 *  Created on: Sep 18, 2024
 *      Author: arthur
 */

#include "mode.h"
#if SLOW_TX_MODE

#ifdef __cplusplus
extern "C" {
#endif

 void slow_tx_main(
					UART_HandleTypeDef* p_huart,
					DMA_HandleTypeDef* p_hdma_tim2_up,
					TIM_HandleTypeDef* p_htim2);

#ifdef __cplusplus
}
#endif

#endif
