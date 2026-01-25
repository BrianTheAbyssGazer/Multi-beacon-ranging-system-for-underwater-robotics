/*
 * peak_detector_main.h
 *
 *  Created on: Sep 11, 2024
 *      Author: arthur
 */

#include "mode.h"
#if BASIC_PEAK_DETECTOR_MODE

#ifdef __cplusplus



extern "C" {
#endif

void peak_detector_main(ADC_HandleTypeDef*, TIM_HandleTypeDef*, UART_HandleTypeDef*, OPAMP_HandleTypeDef*, OPAMP_HandleTypeDef*);

#ifdef __cplusplus
}
#endif

#endif