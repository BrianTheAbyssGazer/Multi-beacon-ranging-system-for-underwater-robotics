/*
 * index_info_rx.cpp
 *
 *  Created on: Sep 11, 2024
 *      Author: arthur
 */


#include "main.h"
#include "index_info_tx.h"
#include "global_buffer_def.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

#include "mavlink/common/mavlink.h"

#pragma GCC diagnostic pop
IndexInfoTX :: IndexInfoTX(UART_HandleTypeDef* p_huart) {
    this->p_huart = p_huart;
}


// Send an index packet, which contains peak index, index prefix and peak value.
// LSBys sent first.
/*
Args:
 - buffer index
 - index prefix
 - peak value (at index)
*/

void IndexInfoTX :: stream_adc(int adc_val) {
	static uint8_t bytes[5]={INFO_2,0,0,0,0};
	bytes[1] = static_cast<uint8_t>(adc_val >> 0);
	bytes[2] = static_cast<uint8_t>(adc_val >> 8);
	bytes[3] = static_cast<uint8_t>(adc_val >> 16);
	bytes[4] = static_cast<uint8_t>(adc_val >> 24);
	HAL_UART_Transmit(p_huart, bytes, 5, 0xFFFF);
}
void IndexInfoTX :: stream_adc(uint16_t adc_val) {
	static uint8_t bytes[3]={INFO_2,0,0};
	bytes[1] = static_cast<uint8_t>(adc_val >> 0);
	bytes[2] = static_cast<uint8_t>(adc_val >> 8);
	HAL_UART_Transmit(p_huart, bytes, 3, 0xFFFF);
}
void IndexInfoTX :: send_byte(uint8_t byte) {
	static uint8_t bytes[2]={INFO_1,0};
	bytes[1] = byte;
	HAL_UART_Transmit(p_huart, bytes, 2, 0xFFFF);
}
void IndexInfoTX :: send_bytes(uint8_t* data) {
	static uint8_t bytes[STRING_LEN+5];
	bytes[0] = INFO_3;
	for(size_t i=0;i<STRING_LEN+4;i++)bytes[i+1]=data[i];
	HAL_UART_Transmit(p_huart, bytes, STRING_LEN+5, 0xFFFF);
}
void IndexInfoTX :: send_range_and_depth(uint8_t sensor_id, uint16_t range, uint16_t depth) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    mavlink_message_t msg;
    uint16_t len;
    static const float zero_quaternion[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    // -------------------------------------------------------------
    // 1. Pack DISTANCE_SENSOR (ID & distance)
    // -------------------------------------------------------------
    mavlink_msg_distance_sensor_pack(
        1,                             // 1.  uint8_t system_id
        MAV_COMP_ID_PATHPLANNER,       // 2.  uint8_t component_id
        &msg,                          // 3.  mavlink_message_t* msg
        0,                             // 4.  uint32_t time_boot_ms
        0,                            // 5.  uint16_t min_distance (cm)
        5000,                          // 6.  uint16_t max_distance (cm)
		range,           // 7.  uint16_t current_distance (cm)
        MAV_DISTANCE_SENSOR_ULTRASOUND,// 8.  uint8_t type
        sensor_id,                     // 9.  uint8_t id
        MAV_SENSOR_ROTATION_PITCH_270, // 10. uint8_t orientation (Facing Down)
        255,                           // 11. uint8_t covariance (255 = unknown)
        0.0f,                          // 12. float horizontal_fov (rad, 0 = N/A)
        0.0f,                          // 13. float vertical_fov (rad, 0 = N/A)
        zero_quaternion,               // 14. const float* quaternion (Must be valid array pointer!)
        0                              // 15. uint8_t signal_quality (0% = unknown, 1-100%)
    );
    len = mavlink_msg_to_send_buffer(buf, &msg);
    HAL_UART_Transmit(p_huart, buf, len, HAL_MAX_DELAY);
    // -------------------------------------------------------------
    // 2. Pack DEPTH_SENSOR (ID & depth)
    // -------------------------------------------------------------
    mavlink_msg_distance_sensor_pack(
        2,                             // 1.  uint8_t system_id
        MAV_COMP_ID_PATHPLANNER,       // 2.  uint8_t component_id
        &msg,                          // 3.  mavlink_message_t* msg
        0,                             // 4.  uint32_t time_boot_ms
        0,                            // 5.  uint16_t min_distance (cm)
        5000,                          // 6.  uint16_t max_distance (cm)
		depth,           // 7.  uint16_t current_distance (cm)
        MAV_DISTANCE_SENSOR_ULTRASOUND,// 8.  uint8_t type
        sensor_id,                     // 9.  uint8_t id
		MAV_SENSOR_ROTATION_PITCH_90, // 10. uint8_t orientation (Facing Down)
        255,                           // 11. uint8_t covariance (255 = unknown)
        0.0f,                          // 12. float horizontal_fov (rad, 0 = N/A)
        0.0f,                          // 13. float vertical_fov (rad, 0 = N/A)
        zero_quaternion,               // 14. const float* quaternion (Must be valid array pointer!)
        0                              // 15. uint8_t signal_quality (0% = unknown, 1-100%)
    );
    len = mavlink_msg_to_send_buffer(buf, &msg);
    HAL_UART_Transmit(p_huart, buf, len, HAL_MAX_DELAY);
}
//Send error1 packet
/*
Args:
 - buffer index
 - index prefix
*/
void IndexInfoTX :: transmit_err_1(int buf_idx, int pre_idx) {
    send_buf[0] = IdxInfoHeader::ERR_1;
    *((uint16_t*)(send_buf + 1)) = (uint16_t) buf_idx;
    *((uint16_t*)(send_buf + 3)) = (uint16_t) pre_idx;
    send_buf[5] = 0;
    send_buf[6] = 0;
	HAL_UART_Transmit_IT(p_huart, send_buf, IITX_PACKET_LEN);
}



// For debugging
void IndexInfoTX :: test_pattern() {
    int i=0;
    int j=0;
    uint16_t k = 0;
    while (1) {
        transmit_idx(i, j, k);
        HAL_Delay(100);
        transmit_err_1(0,0);
        HAL_Delay(400);
        i+=1;
        j+=2;
        k+=3;
        if (i >= 24000) i = 0;
        if (j >= (1<<15)) j = 0;
        if (k >= (1<<12)) k = 0;
    }
}
