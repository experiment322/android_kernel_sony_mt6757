#ifndef __PL_NVRAM_H__
#define __PL_NVRAM_H__

#include "pl_calibration.h"

#define MAX_RETRY_COUNT 10

void init_pl_nvram(STK3X1X_THRESHOLD *stk3x1x_threshold);

int read_pl_nvram(STK3X1X_THRESHOLD *stk3x1x_threshold);

int write_back_to_driver(STK3X1X_THRESHOLD *stk3x1x_threshold);

#endif