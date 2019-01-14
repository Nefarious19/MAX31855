/*
 * MAX31855.c
 *
 *  Created on: 13.01.2019
 *      Author: rafal
 */

#include "max31855.h"

#define MAX31855_DATA_SIZE 4

static uint8_t MAX31855_dataBuffer[MAX31855_DATA_SIZE] = { 0 };

#if USE_RETURN_AS_STRING_FEATURE == 1
static const char MAX31855_error[] = "ERROR CODE: ";
#endif

static MAX31855_STATUS_t internalSTATUS =
{
		.dataStatus = MAX31855_DATA_NOT_REFRESHED,
		.chipStatus = MAX31855_STATUS_NO_FAULT,
		.libStatus = MAX31855_LIB_NOT_INITIALIZED
};

static MAX31855_TCTemp_t internalTCTemp = {
		.tempDec = 0xFFFF,
		.tempFract = 0xFF
};
static MAX31855_InternalTemp_t InternalTemp = {
		.tempDec = 0xFF,
		.tempFract = 0xFFFF
};

void (*spiReadNBytesFuncPtrInternal)(uint8_t * data, uint8_t len);

MAX31855_STATUS_t MAX31855_init(void (*spiReadNBytesFuncPtr)(uint8_t * data, uint8_t len))
{
	if(spiReadNBytesFuncPtr)
	{
		spiReadNBytesFuncPtrInternal = spiReadNBytesFuncPtr;

		internalSTATUS.dataStatus = MAX31855_DATA_NOT_REFRESHED;
		internalSTATUS.chipStatus = MAX31855_STATUS_NO_FAULT;
		internalSTATUS.libStatus = MAX31855_LIB_INITIALIZED;

		return internalSTATUS;
	}
	else
	{
		internalSTATUS.dataStatus = MAX31855_DATA_NOT_REFRESHED;
		internalSTATUS.chipStatus = MAX31855_STATUS_NO_FAULT;
		internalSTATUS.libStatus = MAX31855_LIB_NOT_INITIALIZED;

		return internalSTATUS;
	}
}

MAX31855_STATUS_t MAX31855_getDataFromChip(void)
{
	if(spiReadNBytesFuncPtrInternal && (internalSTATUS.libStatus == MAX31855_LIB_INITIALIZED))
	{
		spiReadNBytesFuncPtrInternal(MAX31855_dataBuffer, MAX31855_DATA_SIZE);

		internalSTATUS.dataStatus = MAX31855_DATA_REFRESHED;
		internalSTATUS.chipStatus = MAX31855_STATUS_NO_FAULT;
		internalSTATUS.libStatus = MAX31855_LIB_INITIALIZED;

		if( MAX31855_dataBuffer[1] & 0x01 )
		{
			uint8_t faultCode = MAX31855_dataBuffer[3] & 0x07;

			switch(faultCode)
			{
				case 0x01:
					internalSTATUS.chipStatus = MAX31855_STATUS_OC_FAULT;
			    break;
				case 0x02:
					internalSTATUS.chipStatus = MAX31855_STATUS_SCG_FAULT;
				break;
				case 0x04:
					internalSTATUS.chipStatus = MAX31855_STATUS_SCV_FAULT;
				break;
			}

		}
		else
		{


			internalTCTemp.tempDec = (int16_t)((0x00FF & MAX31855_dataBuffer[1]) | (0xFF00 & (MAX31855_dataBuffer[0]<<8)))/4;
			internalTCTemp.tempFract = internalTCTemp.tempDec % 4;
			if(internalTCTemp.tempDec & (1<<15)) internalTCTemp.tempFract *= (-1);
			internalTCTemp.tempDec /= 4;
			internalTCTemp.tempFract = (internalTCTemp.tempFract * 25);

			InternalTemp.tempDec = MAX31855_dataBuffer[2];
			InternalTemp.tempFract = (MAX31855_dataBuffer[3]>>4) & 0x000F;
			if(InternalTemp.tempDec & (1<<7)) InternalTemp.tempFract *= (-1);
			InternalTemp.tempFract = 625 * InternalTemp.tempFract;

			if(internalTCTemp.tempDec > 1600 || internalTCTemp.tempDec < -250 || InternalTemp.tempDec > 127 || InternalTemp.tempDec < -55)
			{
				internalSTATUS.chipStatus = MAX31855_STATUS_WRONG_DATA_FAULT;
			}
		}

		return internalSTATUS;
	}
	else
	{
		internalSTATUS.dataStatus = MAX31855_DATA_NOT_REFRESHED;
		internalSTATUS.chipStatus = MAX31855_STATUS_NO_FAULT;
		internalSTATUS.libStatus = MAX31855_LIB_NOT_INITIALIZED;

		return internalSTATUS;
	}
}

MAX31855_STATUS_t MAX31855_getStatus(void)
{
	return internalSTATUS;
}


MAX31855_STATUS_t MAX31855_getTCTemperature(MAX31855_TCTemp_t * data)
{
	if( internalSTATUS.dataStatus == MAX31855_DATA_REFRESHED   &&
	    internalSTATUS.chipStatus == MAX31855_STATUS_NO_FAULT  &&
	    internalSTATUS.libStatus  == MAX31855_LIB_INITIALIZED
	)
	{
		data->tempDec = internalTCTemp.tempDec;
		data->tempFract = internalTCTemp.tempFract;
		return internalSTATUS;
	}
	else
	{
		data->tempDec = 0;
		data->tempFract = 0u;
		return internalSTATUS;
	}


}

MAX31855_STATUS_t MAX31855_getInternalTemperature(MAX31855_InternalTemp_t * data)
{
	if( internalSTATUS.dataStatus == MAX31855_DATA_REFRESHED   &&
		internalSTATUS.chipStatus == MAX31855_STATUS_NO_FAULT  &&
		internalSTATUS.libStatus  == MAX31855_LIB_INITIALIZED
	)
	{

		data->tempDec = InternalTemp.tempDec;
		data->tempFract = InternalTemp.tempFract;
		return internalSTATUS;
	}
	else
	{
		data->tempDec = 0;
		data->tempFract = 0u;
		return internalSTATUS;
	}
}

#if USE_RETURN_AS_STRING_FEATURE == 1

char * MAX31855_getTCTemperatureAsString(char * data, uint8_t precision)
{
	char * ptr = data;
	MAX31855_STATUS_t state;
	MAX31855_TCTemp_t temp;
	state = MAX31855_getTCTemperature(&temp);

	if( state.chipStatus == MAX31855_STATUS_NO_FAULT)
	{
		uint16_t divider = 1;


		if(temp.tempDec & (1<<15))
		{
			*ptr++ = '-';
			temp.tempDec = (~temp.tempDec) + 1;
		}

		while(temp.tempDec/divider >= 10)
		{
			divider *= 10;
		}

		while( divider )
		{
			*ptr++ = (temp.tempDec / divider) + '0';
			temp.tempDec = temp.tempDec%divider;
			divider /= 10;
		}

		if(precision)
		{
			*ptr++ = '.';
			precision = precision > 2 ? 2 : precision;
			if(temp.tempFract == 0)
			{
				while(precision)
				{
					*ptr++ = '0';
					precision--;
				}
			}
			else
			{
				*ptr++ = ((precision == 1) && (temp.tempFract/10 > 4)) ? temp.tempFract / 10 + 1 + '0' :   temp.tempFract / 10 + '0';
				precision--;
				if(precision) *ptr++ = temp.tempFract % 10 + '0';
			}
		}

		*ptr = 0;

	}
	else
	{
		uint8_t idx = 0;

		while( MAX31855_error[idx] )
		{
			*ptr++ = MAX31855_error[idx++];
		}

		*ptr++ = (char)((uint8_t)(state.chipStatus)+'0');
		*ptr = 0;
	}

	return data;
}

char * MAX31855_getInternalTemperatureAsString(char * data, uint8_t precision)
{
	char * ptr = data;

	MAX31855_STATUS_t state;

	MAX31855_InternalTemp_t temp;

	state = MAX31855_getInternalTemperature(&temp);

	if( state.chipStatus == MAX31855_STATUS_NO_FAULT)
	{
		uint16_t divider = 1;

		if(temp.tempDec & (1<<7))
		{
			*ptr++ = '-';
			temp.tempDec = (~temp.tempDec) + 1;
		}

		while(temp.tempDec/divider >= 10)
		{
			divider *= 10;
		}

		while( divider )
		{
			*ptr++ = (temp.tempDec / divider) + '0';
			temp.tempDec = temp.tempDec%divider;
			divider /= 10;
		}

		if(precision)
		{
			*ptr++ = '.';
			precision = precision > 3 ? 4 : precision;

			if(temp.tempFract == 0)

			{
				while(precision)
				{
					*ptr++ = '0';
					precision--;
				}
			}
			else
			{
				divider = 1000;
				while(divider)
				{
					*ptr++ = ((precision == 1) && (temp.tempFract/divider > 4)) ? temp.tempFract/divider + 1 + '0' : temp.tempFract/divider + '0' ;
					temp.tempFract = temp.tempFract % divider;
					divider /= 10;
					precision--;
					if(!precision) break;
				}
			}
		}
		*ptr = 0;
	}
	else
	{
		uint8_t idx = 0;

		while( MAX31855_error[idx] )
		{
			*ptr++ = MAX31855_error[idx++];
		}

		*ptr++ = (char)((uint8_t)(state.chipStatus)+'0');
		*ptr = 0;
	}
	return data;
}
#endif
