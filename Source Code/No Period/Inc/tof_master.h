/*
 * tof_master.h
 *
 *  Created on: Sep 18, 2024
 *      Author: arthur
 */


#include "mode.h"
#if TIME_OF_FLIGHT_MODE

#ifdef __cplusplus



extern "C" {
#endif

void tof_master_main(ADC_HandleTypeDef*,
                    TIM_HandleTypeDef*,
                    UART_HandleTypeDef*,
                    OPAMP_HandleTypeDef*,
                    OPAMP_HandleTypeDef*,
                    DMA_HandleTypeDef*,
                    TIM_HandleTypeDef*);

#ifdef __cplusplus
}
#endif

#endif
