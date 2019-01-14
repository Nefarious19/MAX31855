/*
 * max31855.h
 *
 *  Created on: 13.01.2019
 *      Author: rafal
 */

#ifndef MAX31855_H_
#define MAX31855_H_

#include <stdint.h>

//set this to zero if you don't want to use this feature
#define USE_RETURN_AS_STRING_FEATURE 1 //0

//temperature of thermocouple
typedef struct
{
	int16_t tempDec;		//signed int part of measured temperature
	uint8_t tempFract;      //fractional part of measured temperature
}MAX31855_TCTemp_t;

//internal chip temperature
typedef struct
{
	int8_t tempDec;			//signed int part of measured temperature
	uint16_t tempFract;     //fractional part of measured temperature
}MAX31855_InternalTemp_t;

//chip fault codes
typedef enum
{
	MAX31855_STATUS_NO_FAULT,  //no fault detected
	MAX31855_STATUS_OC_FAULT,  //no thermocouple connected fault
	MAX31855_STATUS_SCG_FAULT, //thermocuple shorted to GND fault
	MAX31855_STATUS_SCV_FAULT,  //thermocuple shorted to VCC fault
	MAX31855_STATUS_WRONG_DATA_FAULT
}MAX31855_ChipStatus_t;

//library status
typedef enum
{
	MAX31855_LIB_NOT_INITIALIZED,
	MAX31855_LIB_INITIALIZED,
}MAX31855_libStatus_t;

//data status
typedef enum
{
	MAX31855_DATA_NOT_REFRESHED,
	MAX31855_DATA_REFRESHED
}MAX31855_dataStatus_t;

//ale states merged into one struct
typedef struct
{
	MAX31855_ChipStatus_t chipStatus;
	MAX31855_libStatus_t  libStatus;
	MAX31855_dataStatus_t dataStatus;
}MAX31855_STATUS_t;

/*
 * @bief: This function initializes library
 * @input: pointer to the function which fills given buffer with data from SPI
 * @output: state of library
 */
MAX31855_STATUS_t MAX31855_init(void (*spiReadNBytesFuncPtr)(uint8_t * data, uint8_t len));

/*
 * @bief: This function returns current state of library
 * @output: state of library
 */
MAX31855_STATUS_t MAX31855_getStatus(void);

/*
 * @bief: This function refreshes data from MAX31855 via SPI protocol
 *        good idea is to invoke this function with maximum period of 100ms
 *        which is data refresh rate of MAX31855
 * @output: state of library
 */
MAX31855_STATUS_t MAX31855_getDataFromChip(void);

/*
 * @bief: This function is used to get last measured thermocouple temperature
 *        MAX31855_STATUS_t MAX31855_getDataFromChip(void) must be invoked at
 *        least once to get any data.
 *
 * @output: state of library
 */
MAX31855_STATUS_t MAX31855_getTCTemperature(MAX31855_TCTemp_t * data);

/*
 * @bief: This function is used to get last measured internal MAX31855 temperature
 *        MAX31855_STATUS_t MAX31855_getDataFromChip(void) must be invoked at
 *        least once to get any data.
 *
 * @output: state of library
 */
MAX31855_STATUS_t MAX31855_getInternalTemperature(MAX31855_InternalTemp_t * data);

#if USE_RETURN_AS_STRING_FEATURE == 1

/*
 * @bief: This function is used to get last measured thermocouple temperature
 *        in string format.
 * @input data - pointer to the char buffer
 * @input precision - value from 0 to 2. Number of precision of fractional part
 *
 * @output: pointer to the same filled char buffer
 */
char * MAX31855_getTCTemperatureAsString(char * data, uint8_t precision);

/*
 * @bief: This function is used to get last measured internal temperature
 *        in string format.
 * @input data - pointer to the char buffer
 * @input precision - value from 0 to 2. Number of precision of fractional part
 *
 * @output: pointer to the same filled char buffer
 */
char * MAX31855_getInternalTemperatureAsString(char * data, uint8_t precision);

#endif

#endif /* MAX31855_H_ */
