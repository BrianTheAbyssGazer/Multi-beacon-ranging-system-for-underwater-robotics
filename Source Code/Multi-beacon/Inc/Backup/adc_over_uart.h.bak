
#ifdef __cplusplus

#include "pga.h"
#include "cmd_rx.h"

#define SIGNPOST_LEN 10
#define START_SIGNAL 1<<7
#define STOP_SIGNAL  1<<6


class ADC_over_UART {
    public:
        ADC_HandleTypeDef* p_hadc;
        UART_HandleTypeDef* p_huart;
        static volatile int buffer_full_flag;
        uint8_t start_signpost[SIGNPOST_LEN];
        uint8_t stop_signpost[SIGNPOST_LEN];
        PGA_cascade_2 pgas;
        CMD_RX cmd_rx;

        ADC_over_UART(ADC_HandleTypeDef*, TIM_HandleTypeDef*, UART_HandleTypeDef*, OPAMP_HandleTypeDef*, OPAMP_HandleTypeDef*);
        void main(void);
};


extern "C" {
#endif

void adc_over_uart(ADC_HandleTypeDef*, TIM_HandleTypeDef*, UART_HandleTypeDef*, OPAMP_HandleTypeDef*, OPAMP_HandleTypeDef*);


//Called when first half of buffer is filled
//void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef*);

//Called when buffer is completely filled
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef*);

#ifdef __cplusplus
}
#endif
