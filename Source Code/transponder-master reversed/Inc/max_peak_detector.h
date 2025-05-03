/*
 * max_peak_detector.h
 *
 *  Created on: Sep 11, 2024
 *      Author: arthur
 */


#ifdef __cplusplus

#include "index_info_tx.h"



class Timestamp { 
  public:
    int idx;
    int pfx;

    Timestamp(int, int);
    int get_total(void);
    static Timestamp from_total(int);
};

enum MPDState {
    IDLE, 
    PROC_BUF_1ST_HLF,
    PROC_BUF_2ND_HLF,
    ERROR_1,
};


enum MPDSearchState {
    FIND_SIGNAL,
    FIND_WINDOW_MAX,
    DEAD_ZONE,
};


class MaxPeakDetector {
    //hardware ----
    private:    
        ADC_HandleTypeDef* p_hadc;
        TIM_HandleTypeDef* p_htim;
        IndexInfoTX* p_index_info_tx;
    
    
    //states -------
    public:
        static volatile int global_state; 
    private:
        int search_sub_state;

    // Parameters ---------
    public:
        bool min_aid; //set to true to incorporate min peak assistance
        int search_threshold_reduction;
        int search_window;
        int dead_zone_len; //set to -1 to jump to buffer end after each peak detection
        int search_threshold; // this is also dynamically updated
        
    //search context --------
    private:
        int last_peak_val; // the value of the last successfully detected pulse peak
        int last_peak_idx;
        int last_peak_pfx;
        int tentative_max_val;
        int tentative_max_idx;
        int tentative_max_pfx;
        int tentative_min_val;
        int tentative_min_idx;
        int tentative_min_pfx;
        int window_count;
        int dead_zone_count;
        
    public:
        int cur_idx; //current idx of adc buffer
        static volatile int cur_pfx; // incremented each time the ADC buffer completely fills

    
    // methods -------
    public:
        MaxPeakDetector(ADC_HandleTypeDef*, TIM_HandleTypeDef*, IndexInfoTX*);
        Timestamp detect_peak(void);
        void send_data2computer(int d_pfx, int end_idx, uint16_t data);
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

