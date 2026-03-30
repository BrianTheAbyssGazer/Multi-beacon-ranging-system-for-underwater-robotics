/*
 * max_peak_detector.h
 *
 *  Created on: Sep 11, 2024
 *      Author: arthur
 */


#ifdef __cplusplus

#include "index_info_tx.h"
#include <deque>
#include "global_buffer_def.h"



class Timestamp {
  public:
	uint16_t idx;
	uint16_t pfx;

    Timestamp(uint16_t, uint16_t);
    uint16_t get_total(void);
    static Timestamp from_total(uint16_t);
};

enum MPDState {
    PROC_BUF_1ST_HLF,
    PROC_BUF_2ND_HLF,
    IDLE,
    ERROR_1,
};


enum MPDSearchState {
    NO_SIGNAL,
    YES_SIGNAL,
	DEMODULATOR_DISABLED,
};


class MaxPeakDetector {
    //hardware ----
    private:
        ADC_HandleTypeDef* p_hadc;
        TIM_HandleTypeDef* p_htim;
        IndexInfoTX* p_index_info_tx;


    //states -------
    public:
        static volatile uint16_t global_state;
    private:
        uint16_t search_sub_state;

    // Parameters ---------
    public:
        static bool sending_signal;
        uint16_t search_threshold_reduction;
        uint16_t search_window;
        uint16_t dead_zone_len; //set to -1 to jump to buffer end after each peak detection
        uint16_t search_threshold;   //search context --------
        bool signal_flag;
        bool data_flag;
		uint16_t last_peak_val; // the value of the last successfully detected pulse peak
		uint16_t last_peak_idx;
		uint16_t last_peak_pfx;

    private:
		uint16_t enable_pfx;
		uint16_t enable_idx;
		uint16_t pinout_idx;
		uint16_t pinout_pfx;
		uint16_t disable_pfx;
		uint16_t disable_idx;
		uint32_t delta_idx;
		uint16_t tentative_max_val;
		uint16_t tentative_max_idx;
		uint16_t tentative_max_pfx;
		uint16_t tentative_min_val;
		uint16_t tentative_min_idx;
		uint16_t tentative_min_pfx;
		uint16_t window_count;
		uint16_t dead_zone_count;
#if DECODE
        float phase;
        uint32_t phase_int;
        uint8_t sample_counter;
        uint8_t symbol_counter;
        float corr_sum;
#elif DEBUG_TIM
        uint16_t pre_val;
        uint16_t corr_sum;
        uint8_t sample_counter;
        uint8_t symbol_counter;
#endif
    public:
        uint16_t cur_idx; //current idx of adc buffer
        uint16_t uart_idx; //current idx of adc buffer
        uint16_t ccm_idx; //current idx of adc buffer
        uint16_t bg_idx; //current idx of adc buffer
        static volatile uint16_t cur_pfx; // incremented each time the ADC buffer completely fills
#if DECODE
        uint8_t rx_data[STRING_LEN+4];
        uint8_t inverse_data=0;
#endif
    // methods -------
    public:
        MaxPeakDetector(ADC_HandleTypeDef*, TIM_HandleTypeDef*, IndexInfoTX*);
        void detect_peak(void);
        void send_data2computer(uint16_t d_pfx, uint16_t end_idx, uint16_t data);
        void mark_pinout(uint16_t pfx, uint16_t idx);
    private:
        void search_loop(void);
        void error_1_handle();
};




extern "C" {
    #endif

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* p_hadc);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* p_hadc);


    #ifdef __cplusplus
}

#endif
