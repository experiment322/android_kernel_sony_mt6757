#include <stdio.h>
#include <stdlib.h>

#include "pl_misc_ta.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PL_Calibration"

int read_TA_ready_retry = 0;

static const char* miscta_strerror(miscta_status_t err)
{
    switch (err) {
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

int Read_TA_part(STK3X1X_THRESHOLD *stk3x1x_threshold)
{
    miscta_status_t rval;
    uint32_t i;
    int read_ta_result = 0;
    uint32_t size = 9;
    //uint8_t *buf = malloc(size);
    char buf[9] = {0};
    int HT = 0, LT = 0,CT = 0, Test = 0;

    if ((rval = miscta_read_unit(TA_CAP_PROXIMITY_CALIBRATION_DATA, buf, &size)) != MT_SUCCESS) {
        ALOGE("STK Operation returned error code %d (%s).\n", rval, miscta_strerror(rval));
        printf("STK Operation returned error code %d (%s).\n", rval, miscta_strerror(rval));

        return 0;
    }

    ALOGI("STK Address %d, size %d, data:\n", TA_CAP_PROXIMITY_CALIBRATION_DATA, size);
    printf("\nSTK Address %d, size %d, data:\n", TA_CAP_PROXIMITY_CALIBRATION_DATA, size);

    if (size == 9) {
        for (i = 0; i < size; i++) {
            ALOGI("STK i = %d,  %02X    %d \n", i, buf[i], buf[i]);
            printf("STK i = %d,  %02X    %d \n", i, buf[i], buf[i]);
        }

        if (buf[8] == 231) {
            HT = (buf[0] << 8) | buf[1];
            LT = (buf[2] << 8) | buf[3];
            CT = (buf[4] << 8) | buf[5];
            Test = (buf[6] << 8) | buf[7];

            if ((CT > 0) && (CT < 1000)) {
                stk3x1x_threshold->ps_high_thd = HT;
                stk3x1x_threshold->ps_low_thd = LT;
                stk3x1x_threshold->result_ct = CT;
                stk3x1x_threshold->result_ps_cover = Test;

                stk3x1x_threshold->result_als_value = 0;
                stk3x1x_threshold->cci_transmittance_cali = 900;
                stk3x1x_threshold->cci_als_adc_cali = 0;
                stk3x1x_threshold->cci_als_lux_cali = 0;
                stk3x1x_threshold->cci_als_adc_test = 0;
                read_ta_result = 1;
            }
        } else {
            ALOGE("[STK]Magic number not correct buf[8] = %d\n", buf[8]);
            printf("\n[STK]Magic number not correct buf[8] = %d\n", buf[8]);
        }

        ALOGI("[STK]TA HT = %d LT = %d CT = %d Test = %d\n", HT, LT, CT, Test);
        ALOGI("[STK]TA ps_high_thd = %d\n", (buf[0] << 8) | buf[1]);
        ALOGI("[STK]TA ps_low_thd = %d\n", (buf[2] << 8) | buf[3]);
        ALOGI("[STK]TA result_ct = %d\n", (buf[4] << 8) | buf[5]);
        ALOGI("[STK]TA result_ps_cover = %d\n", (buf[6] << 8) | buf[7]);
        ALOGI("[STK]TA cci_als_lux_cali = %d\n", stk3x1x_threshold->cci_als_lux_cali);
        ALOGI("[STK]TA cci_transmittance_cali = %d\n", stk3x1x_threshold->cci_transmittance_cali);
        ALOGI("[STK]TA cci_als_adc_cali = %d\n", stk3x1x_threshold->cci_als_adc_cali);
        ALOGI("[STK]TA result_als_value = %d\n", stk3x1x_threshold->result_als_value);
        ALOGI("[STK]TA cci_als_adc_test = %d\n", stk3x1x_threshold->cci_als_adc_test);
        ALOGI("[STK]Magic number buf[8] = %d\n", buf[8]);
        ALOGI("[STK]TA read_ta_result = %d\n", read_ta_result);

        printf("[STK]TA HT = %d LT = %d CT = %d Test = %d\n", HT, LT, CT, Test);
        printf("\n[STK]TA ps_high_thd = %d\n", (buf[0] << 8) | buf[1]);
        printf("[STK]TA ps_low_thd = %d\n", (buf[2] << 8) | buf[3]);
        printf("[STK]TA result_ct = %d\n", (buf[4] << 8) | buf[5]);
        printf("[STK]TA result_ps_cover = %d\n", (buf[6] << 8) | buf[7]);

        printf("[STK]TA cci_als_lux_cali = %d\n", stk3x1x_threshold->cci_als_lux_cali);
        printf("[STK]TA cci_transmittance_cali = %d\n", stk3x1x_threshold->cci_transmittance_cali);
        printf("[STK]TA cci_als_adc_cali = %d\n", stk3x1x_threshold->cci_als_adc_cali);
        printf("[STK]TA result_als_value = %d\n", stk3x1x_threshold->result_als_value);
        printf("[STK]TA cci_als_adc_test = %d\n", stk3x1x_threshold->cci_als_adc_test);
        printf("\n[STK]Magic number buf[8] = %d\n", buf[8]);
        printf("[STK]TA read_ta_result = %d\n", read_ta_result);
    }

    return read_ta_result;
}