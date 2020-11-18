#ifndef __PL_CALIBRATION_H__
#define __PL_CALIBRATION_H__

#include <log/log.h>

typedef struct{
    unsigned int result_ct;
    unsigned int result_ps_cover;
    unsigned int result_als_value;
    unsigned int cci_als_adc_test;
    unsigned int cci_transmittance_cali;    /* add for FTM ALS CALI*/
    unsigned int cci_als_adc_cali;          /* add for FTM ALS CALI*/
    unsigned int cci_als_lux_cali;
    unsigned int ps_high_thd;
    unsigned int ps_low_thd;
} STK3X1X_THRESHOLD;

#endif