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

#include "pl_nvram.h"
#define USE_MISCTA
#ifdef USE_MISCTA
#include "pl_misc_ta.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PL_Calibration"

int read_nvram_ready_retry = 0;

/* add for read PL sensor data and write to Driver end */
int main(void)
{
    ALOGI("Read PL sensor data and write to Driver\n");
    int res = 0;
    STK3X1X_THRESHOLD stk3x1x_threshold;

#ifdef USE_MISCTA
    ALOGI("Read MiscTA\n");
    res = Read_TA_part(&stk3x1x_threshold);
    if (res == 0) {
#endif
        ALOGI("Read NVRAM\n");
        init_pl_nvram(&stk3x1x_threshold);
        res = read_pl_nvram(&stk3x1x_threshold);

#ifdef USE_MISCTA
    }
#endif

    if (res) {
        ALOGI("Write back to driver\n");
        res = write_back_to_driver(&stk3x1x_threshold);
    } else {
        ALOGE("Read PL sensor data fail\n");
        printf("[STK]read NVRAM fail\n");
    }

    return 0;
}