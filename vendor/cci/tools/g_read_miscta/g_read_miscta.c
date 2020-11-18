
// Add own daemon for G-sensor
// SoMC MiscTA address is 4963

#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <fcntl.h>
#include <string.h>
#include <cutils/properties.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <android/log.h>

#include "./inc/miscta.h"
#include "./inc/miscta_wrappers.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , "G_MISCTA",__VA_ARGS__)
#define TA_CAP_G_SENSOR_CALIBRATION_DATA	4963
#define GRAVITY_EARTH_1000           9807	/* about (9.80665f)*1000 */
int g_iGsenorCali[3];
char g_strSysfsPath[200];

static const char* miscta_strerror(miscta_status_t err) 
{
	switch (err)
	{
		case MT_SUCCESS:
			return "SUCCESS";
		case MT_ERROR:
			return "ERROR";
		case MT_NOMEM:
			return "NOMEM";
		case MT_TAERR:
			return "TAERROR";
		case MT_TOOBIG:
			return "TOOBIG";
		case MT_NOTFOUND:
			return "NOTFOUND";
		case MT_INVALPARAM:
			return "INVALPARAM";
		default:
			return "UNKNOWN";
	}
}

int gsensor_read_miscTa()
{
	miscta_status_t rval;
	uint32_t i;
	int read_ta_result=0;
	uint32_t iSize = 14;
	char cBuf[20]={0};

	LOGI("[%s] enter.. \n", __FUNCTION__);
	printf("[%s] enter.. \n", __FUNCTION__);

	if ( (rval = miscta_read_unit(TA_CAP_G_SENSOR_CALIBRATION_DATA, cBuf, &iSize)) != MT_SUCCESS )
	{
		LOGI("[%s] Operation returned error code %d (%s).\n", __FUNCTION__, rval, miscta_strerror(rval));
		printf("[%s] Operation returned error code %d (%s).\n", __FUNCTION__, rval, miscta_strerror(rval));
		read_ta_result = 0;
		return read_ta_result;
	}

	LOGI("[%s] BMA253 Address %d, size %d, data:\n", __FUNCTION__, TA_CAP_G_SENSOR_CALIBRATION_DATA, iSize);
	printf("[%s] BMA253 Address %d, size %d, data:\n", __FUNCTION__, TA_CAP_G_SENSOR_CALIBRATION_DATA, iSize);
	if(iSize == 14)
	{
		for (i = 0; i < iSize; i++)
		{
			LOGI("[%s] i=%d,  %#x  %d \n", __FUNCTION__,i ,cBuf[i] ,cBuf[i]);
			printf("[%s] i=%d,  %#x  %d \n", __FUNCTION__,i ,cBuf[i] ,cBuf[i]);
		}

		if( (cBuf[12] != 0x55) || (cBuf[13] != 0xaa) )
		{
			read_ta_result = 0;	//error
			LOGI("[%s] cBuf[12] = %#x,  cBuf[13] = %#x, invalid magic number!!\n", __FUNCTION__, cBuf[12] ,cBuf[13]);
			printf("[%s] cBuf[12] = %#x,  cBuf[13] = %#x, invalid magic number!!\n", __FUNCTION__, cBuf[12] ,cBuf[13]);
			goto error;
		}
		else
		{
			g_iGsenorCali[0] = (cBuf[0] << 24) | (cBuf[1]<<16) | (cBuf[2]<<8) | (cBuf[3]);
			g_iGsenorCali[1] = (cBuf[4] << 24) | (cBuf[5]<<16) | (cBuf[6]<<8) | (cBuf[7]);
			g_iGsenorCali[2] = (cBuf[8] << 24) | (cBuf[9]<<16) | (cBuf[10]<<8) | (cBuf[11]);
			LOGI("[%s] g_iGsenorCali[0] = (%#x),  g_iGsenorCali[1] = (%#x), g_iGsenorCali[2] = (%#x),\n",
				__FUNCTION__, g_iGsenorCali[0], g_iGsenorCali[1], g_iGsenorCali[2]);
			printf("[%s] g_iGsenorCali[0] = (%#x),  g_iGsenorCali[1] = (%#x), g_iGsenorCali[2] = (%#x),\n",
				__FUNCTION__, g_iGsenorCali[0], g_iGsenorCali[1], g_iGsenorCali[2]);

			if( (g_iGsenorCali[0] == 0x0) && (g_iGsenorCali[1] == 0x0) && (g_iGsenorCali[2] == 0x0))
			{
				read_ta_result = 0;	//no calibration
				goto error;
			}
		}
		read_ta_result = 1;
	}

error:
	LOGI("[%s] read_ta_result = %d \n", __FUNCTION__, read_ta_result);
	printf("[%s] read_ta_result = %d \n", __FUNCTION__, read_ta_result);
	return read_ta_result;
}

void gsensor_cali_miscTa()
{
	int ta_result = 0;
	int iFdGsensor = -1;
	int iByte = 0, iRes = 0;
	char acBuffer[500];
	int iCovert[3];

	LOGI("[%s] enter.. \n", __FUNCTION__);
	printf("[%s] enter.. \n", __FUNCTION__);

	ta_result = gsensor_read_miscTa();
	if(ta_result==0)
	{
		//bad miscTa for g-sensor, do nothing.
		LOGI("[%s] g-sensor do nothing...\n", __FUNCTION__);
		printf("[%s] g-sensor do nothing...\n", __FUNCTION__);
		goto Donothing;
	}
	else
	{
		LOGI("[%s] g_iGsenorCali[0] = (%#x),  g_iGsenorCali[1] = (%#x), g_iGsenorCali[2] = (%#x)\n",
			__FUNCTION__, g_iGsenorCali[0], g_iGsenorCali[1], g_iGsenorCali[2]);
		printf("[%s] g_iGsenorCali[0] = (%#x),  g_iGsenorCali[1] = (%#x), g_iGsenorCali[2] = (%#x)\n",
			__FUNCTION__, g_iGsenorCali[0], g_iGsenorCali[1], g_iGsenorCali[2]);
	}

	strcpy(g_strSysfsPath, "/sys/bus/platform/drivers/gsensor/cali_miscTa");
	iFdGsensor = open(g_strSysfsPath, O_RDWR);

	if (iFdGsensor >= 0)
	{
		iCovert[0] = (g_iGsenorCali[0] *GRAVITY_EARTH_1000) / 65536;
		iCovert[1] = (g_iGsenorCali[1] *GRAVITY_EARTH_1000) / 65536;
		iCovert[2] = (g_iGsenorCali[2] *GRAVITY_EARTH_1000) / 65536;
		//iByte = sprintf(acBuffer, "%d %d %d\n", g_iGsenorCali[0], g_iGsenorCali[1], g_iGsenorCali[2]);
		iByte = sprintf(acBuffer, "%d %d %d\n", iCovert[0], iCovert[1], iCovert[2]);
		iRes = write(iFdGsensor, acBuffer, iByte);
		if (iRes >= 0)
		{
			LOGI("[%s] [CEI]Write cali data to %s Success!!\n", __FUNCTION__, g_strSysfsPath);
			printf("[%s] [CEI]Write cali data to %s Success!!\n", __FUNCTION__, g_strSysfsPath);
		}
		else
		{
			LOGI("[%s][CEI]Write cali data to %s Fail res = %d!!\n", __FUNCTION__, g_strSysfsPath, iRes);
			printf("[%s][CEI]Write cali data to %s Fail res = %d!!\n", __FUNCTION__, g_strSysfsPath, iRes);
		}
		close(iFdGsensor);
	}
	else
	{
		LOGI("[%s][CEI]Open %s Fail!!\n", __FUNCTION__, g_strSysfsPath);
		printf("[%s][CEI]Open %s Fail!!\n", __FUNCTION__, g_strSysfsPath);
	}
Donothing:
	LOGI("[%s] g-sensor finish..\n", __FUNCTION__);
	printf("[%s] g-sensor finish..\n", __FUNCTION__);
}

int main(void)
{
	LOGI("[%s] g-sensor calibration data in miscTa start.. \n", __FUNCTION__);
	printf("[%s] g-sensor calibration data in miscTa start.. \n", __FUNCTION__);
	gsensor_cali_miscTa();

	return 0;
}

