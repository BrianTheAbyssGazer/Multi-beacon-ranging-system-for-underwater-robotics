/*
 * pga.h
 *
 *  Created on: Aug 30, 2024
 *      Author: arthur
 */


#ifdef __cplusplus

/*
Represents a PGA on the STMf303re chip
*/
class PGA303
{
    public:
		OPAMP_HandleTypeDef * p_hopamp;
        PGA303(OPAMP_HandleTypeDef*);
        PGA303() : p_hopamp(nullptr) {}
        void setGain(int);

    static void pgaTest(OPAMP_HandleTypeDef*);

};



/*
Represents a cascade of 2 PGAs
*/
class PGA_cascade_2
{ 
    public:
        PGA303 pga1;
        PGA303 pga2;
        PGA_cascade_2(OPAMP_HandleTypeDef*, OPAMP_HandleTypeDef *);
        void setGain(int);
};



extern "C" {
#endif

/* Functions to call in C file */
void pgaTestC(OPAMP_HandleTypeDef*);



#ifdef __cplusplus
}
#endif

