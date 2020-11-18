#ifndef __PL_MISC_TA_H__
#define __PL_MISC_TA_H__

#include "pl_calibration.h"
#include "inc/miscta.h"
#include "inc/miscta_wrappers.h"

#define TA_CAP_PROXIMITY_CALIBRATION_DATA 4971

static const char* miscta_strerror(miscta_status_t err);

int Read_TA_part(STK3X1X_THRESHOLD *stk3x1x_threshold);

#endif