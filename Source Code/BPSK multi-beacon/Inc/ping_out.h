/*
 * ping_out.h
 *
 *  Created on: Sep 13, 2024
 *      Author: arthur
 */

#ifdef __cplusplus

#define BSRR_PC6_SET_MASK 1<<6
#define BSRR_PC6_RESET_MASK 1<<22


enum POState {
    FIRST_HLF_FREE,
    SECND_HLF_FREE,
	PO_IDLE,
	ERROR_2,
};

enum SetState {
    SET_PIN,
    CLEAR,
	PO_DISABLED,
};

/*
* Class for managing the transponder output
*/
class PingOut {
    private:
        DMA_HandleTypeDef* p_hdma_tim2_up;
        TIM_HandleTypeDef* p_htim2;
        IndexInfoTX* p_index_info_tx;
    public:
        static volatile uint16_t po_state;
        static volatile uint16_t cur_out_pfx; // the prefix of the free space on the buffer

    private:
        uint16_t scheduled_idx; //this has already been rounded to out_buf units
        uint16_t clear_offset; //this has already been rounded to out_buf units
        uint16_t scheduled_pfx; // -1 indicates nothing to schedule
        uint16_t cur_idx;
        uint32_t data_idx;
        bool enable_scheduler;
    public:
        bool periodic_schedule_enable;
        static uint8_t set_state;

        static volatile uint16_t schedule_period;
        static volatile bool time_to_schedule_period;
        static volatile bool time_to_schedule_databit;
        static volatile bool time_to_schedule_phase_keying;


        void set();

    public:
        //parameters:
        uint16_t peak_count;
        uint16_t samples_per_half_period; //i.e. IN_LEN/OUT_LEN
        static bool debug; //toggle GPIO on callbacks, set and reset

        //methods:
        PingOut(DMA_HandleTypeDef*, TIM_HandleTypeDef*, IndexInfoTX*);
        void schedule_ping(uint16_t, uint16_t);
        void start_periodic_scheduler(uint16_t);
        void schedule(uint16_t pfx, uint16_t idx);
        uint16_t start_datapacket_scheduler(uint8_t data);
        void update(void);
        bool calculateParity(bool codeword[], const uint8_t positions[], uint8_t size);
    private:
        void enable_pingout();
};


void first_half_written_callback(DMA_HandleTypeDef*);
void secnd_half_written_callback(DMA_HandleTypeDef*);

extern "C" {

#endif


#ifdef __cplusplus
}
#endif
