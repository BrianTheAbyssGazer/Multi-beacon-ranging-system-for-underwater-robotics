/*
 * pga.cpp
 *
 *  Created on: Aug 30, 2024
 *      Author: arthur
 */



#include "main.h"
#include "pga.h"


/* For directly setting the op-amp CSR registers*/
#define MY_GAIN_MASK 0x0003c000
#define MY_GAIN_2    0xfffc3fff
#define MY_GAIN_4    0xfffc7fff
#define MY_GAIN_8    0xfffcbfff
#define MY_GAIN_16   0xfffcffff



PGA303::PGA303(OPAMP_HandleTypeDef* p_hopamp) {
    this->p_hopamp = p_hopamp;
}



/*
Sets the Gain of the op-amp based on the index.
The gain is linear 2^index. Permitted gains are [1,4]

Index  |  Gain
   1   |   2 
   2   |   4 
   3   |   8 
   4   |   16 

Indexes not in [1,4] will be ignored.
*/
void PGA303::setGain(int index) {
    switch (index)
    {
    case 1:
        p_hopamp->Instance->CSR |= MY_GAIN_MASK;
	    p_hopamp->Instance->CSR &= MY_GAIN_2;
        break;
    case 2:
        p_hopamp->Instance->CSR |= MY_GAIN_MASK;
	    p_hopamp->Instance->CSR &= MY_GAIN_4;
        break;
    case 3:
        p_hopamp->Instance->CSR |= MY_GAIN_MASK;
	    p_hopamp->Instance->CSR &= MY_GAIN_8;
        break;
    case 4:
        p_hopamp->Instance->CSR |= MY_GAIN_MASK;
	    p_hopamp->Instance->CSR &= MY_GAIN_16;
        break;
    
    default:
        break;
    }
}


//extern "C" void HAL_Delay(uint32_t);

/*
Cycles through gain settings on the PGA.
*/
void PGA303::pgaTest(OPAMP_HandleTypeDef* p_hopamp) {
    PGA303 opAmp(p_hopamp);

    while (1) {
        opAmp.setGain(1);
        HAL_Delay(5000);
        opAmp.setGain(2);
        HAL_Delay(5000);
        opAmp.setGain(3);
        HAL_Delay(5000);
        opAmp.setGain(4);
        HAL_Delay(5000);
    }
}


extern "C" void pgaTestC(OPAMP_HandleTypeDef* p_hopamp) {
    PGA303::pgaTest(p_hopamp);
}




PGA_cascade_2::PGA_cascade_2(OPAMP_HandleTypeDef* p_opamp_1, OPAMP_HandleTypeDef* p_opamp_2) {
    pga1.p_hopamp = p_opamp_1;
    pga2.p_hopamp = p_opamp_2;
}



/*
Sets the combined Gain of the op-amps based on the index.
The gain is linear 2^index. Permitted gains are [2,8].

Index  |  Gain
   2   |   4 
   3   |   8
   4   |   16 
   5   |   32 
   6   |   64
   7   |   128
   8   |   256

Indexes not in [2,8] will be ignored.
*/
void PGA_cascade_2::setGain(int index) {
    switch (index)
    {
    case 2:
        pga1.setGain(1);
        pga2.setGain(1);
        break;
    case 3:
        pga1.setGain(2);
        pga2.setGain(1);
        break;
    case 4:
        pga1.setGain(2);
        pga2.setGain(2);
        break;
    case 5:
        pga1.setGain(3);
        pga2.setGain(2);
        break;
    case 6:
        pga1.setGain(3);
        pga2.setGain(3);
        break;
    case 7:
        pga1.setGain(4);
        pga2.setGain(3);
        break;
    case 8:
        pga1.setGain(4);
        pga2.setGain(4);
        break;
    
    default:
        break;
    }
}
