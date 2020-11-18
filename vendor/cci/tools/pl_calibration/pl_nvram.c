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

#include "libnvram.h"
#include "CFG_file_lid.h"
//#include "Custom_NvRam_LID.h"
#include "./../../../mediatek/proprietary/custom/smx1/cgen/inc/Custom_NvRam_LID.h"
//#include "CFG_P_SENSOR_THRESHOLD_File.h"
#include "./../../../mediatek/proprietary/custom/smx1/cgen/cfgfileinc/CFG_P_SENSOR_THRESHOLD_File.h"
//#include "CFG_P_SENSOR_THRESHOLD_Default.h"
#include "./../../../mediatek/proprietary/custom/smx1/cgen/cfgdefault/CFG_P_SENSOR_THRESHOLD_Default.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PL_Calibration"

#define MAX_PATH 100
char input_sysfs_path[MAX_PATH];

void init_pl_nvram(STK3X1X_THRESHOLD *stk3x1x_threshold)
{
    stk3x1x_threshold->ps_high_thd = 0;
    stk3x1x_threshold->ps_low_thd = 0;
    stk3x1x_threshold->result_ct = 0;
    stk3x1x_threshold->result_ps_cover = 0;
    stk3x1x_threshold->result_als_value = 0;
    stk3x1x_threshold->cci_als_adc_test = 0;
    stk3x1x_threshold->cci_als_lux_cali = 0;
    stk3x1x_threshold->cci_transmittance_cali = 900;
    stk3x1x_threshold->cci_als_adc_cali = 0;
}

int read_pl_nvram(STK3X1X_THRESHOLD *stk3x1x_threshold)
{
    char nvram_init_val[500];
    bool Nvram_isReady = false;
    int read_nvram_ready_retry = 0;
    int rec_size = 0;
    int	rec_num = 0;
    int	result = 0;
    int nvram_result = 0;
    int fd = -1;
    int err = 0, ret = 0;
    F_ID p_sensor_threshold_fd = {0};
    int file_p_sensor_lid = (AP_CFG_CUSTEOM_FILE_P_SENSOR_THRESHOLD_RESULT_LID);

    FILE_P_SENSOR_THRESHOLD_STRUCT *p_sensor_threshold = NULL;
    p_sensor_threshold = (FILE_P_SENSOR_THRESHOLD_STRUCT *) malloc(sizeof(FILE_P_SENSOR_THRESHOLD_STRUCT));

    //---Read NVDATA data START----------------------------------------------------------
    while (read_nvram_ready_retry < MAX_RETRY_COUNT) {
        read_nvram_ready_retry++;
        property_get("service.nvram_init" , nvram_init_val , NULL);

        if (strcmp(nvram_init_val, "Ready") == 0 || strcmp(nvram_init_val, "Pre_Ready") == 0) {
            ALOGI("nvram_init_val Ready");
            printf("nvram_init_val Ready");
            Nvram_isReady = true;
            break;
        } else {
            ALOGI("nvram_init_val not Ready, sleep 500ms");
            printf("nvram_init_val not Ready, sleep 500ms");
            usleep(500 * 1000);
        }
    }

    if (p_sensor_threshold != NULL && Nvram_isReady) {
        p_sensor_threshold_fd = NVM_GetFileDesc(file_p_sensor_lid, &rec_size, &rec_num, ISREAD);
        result = read(p_sensor_threshold_fd.iFileDesc, p_sensor_threshold, rec_size * rec_num);

        if (result > 0) {

            /* Colby add for STK FTM struct start 20130112 */
            ALOGI("Nvram Deamon read P sensor calibration data successfully\n");
            printf("Nvram Deamon read P sensor calibration data successfully\n");

            stk3x1x_threshold->ps_high_thd = p_sensor_threshold->data[1];    //9
            stk3x1x_threshold->ps_low_thd = p_sensor_threshold->data[2];     //10
            stk3x1x_threshold->result_ct = p_sensor_threshold->data[3];      //1
            stk3x1x_threshold->result_ps_cover = p_sensor_threshold->data[4];//2

            stk3x1x_threshold->cci_als_lux_cali = p_sensor_threshold->data[6];
            stk3x1x_threshold->cci_transmittance_cali = p_sensor_threshold->data[7]; //4
            stk3x1x_threshold->cci_als_adc_cali = p_sensor_threshold->data[8];       //5

            stk3x1x_threshold->result_als_value = p_sensor_threshold->data[9];   //3
            stk3x1x_threshold->cci_als_adc_test = p_sensor_threshold->data[10];  //7

            ALOGI("[STK]NVRAM ps_high_thd = %d\n", stk3x1x_threshold->ps_high_thd);
            ALOGI("[STK]NVRAM ps_low_thd = %d\n", stk3x1x_threshold->ps_low_thd);
            ALOGI("[STK]NVRAM result_ct = %d\n", stk3x1x_threshold->result_ct);
            ALOGI("[STK]NVRAM result_ps_cover = %d\n", stk3x1x_threshold->result_ps_cover);
            ALOGI("[STK]NVRAM cci_als_lux_cali = %d\n", stk3x1x_threshold->cci_als_lux_cali);
            ALOGI("[STK]NVRAM cci_transmittance_cali = %d\n", stk3x1x_threshold->cci_transmittance_cali);
            ALOGI("[STK]NVRAM cci_als_adc_cali = %d\n", stk3x1x_threshold->cci_als_adc_cali);
            ALOGI("[STK]NVRAM result_als_value = %d\n", stk3x1x_threshold->result_als_value);
            ALOGI("[STK]NVRAM cci_als_adc_test = %d\n", stk3x1x_threshold->cci_als_adc_test);
            ALOGI("[STK]NVRAM P_SENSOR_CALIBRATION result= %d\n", p_sensor_threshold->data[5]);
            ALOGI("[STK]NVRAM L_SENSOR_CALIBRATION result= %d\n", p_sensor_threshold->data[11]);

            printf("[STK]NVRAM ps_high_thd = %d\n", stk3x1x_threshold->ps_high_thd);
            printf("[STK]NVRAM ps_low_thd = %d\n", stk3x1x_threshold->ps_low_thd);
            printf("[STK]NVRAM result_ct = %d\n", stk3x1x_threshold->result_ct);
            printf("[STK]NVRAM result_ps_cover = %d\n", stk3x1x_threshold->result_ps_cover);
            printf("[STK]NVRAM cci_als_lux_cali = %d\n", stk3x1x_threshold->cci_als_lux_cali);
            printf("[STK]NVRAM cci_transmittance_cali = %d\n", stk3x1x_threshold->cci_transmittance_cali);
            printf("[STK]NVRAM cci_als_adc_cali = %d\n", stk3x1x_threshold->cci_als_adc_cali);
            printf("[STK]NVRAM result_als_value = %d\n", stk3x1x_threshold->result_als_value);
            printf("[STK]NVRAM cci_als_adc_test = %d\n", stk3x1x_threshold->cci_als_adc_test);
            printf("[STK]NVRAM P_SENSOR_CALIBRATION result= %d\n", p_sensor_threshold->data[5]);
            printf("[STK]NVRAM L_SENSOR_CALIBRATION result= %d\n", p_sensor_threshold->data[11]);

            nvram_result = 1;
        } else {
            ALOGE("[STK]Nvram Deamon read P sensor calibration data FAIL\n");
            printf("[STK]Nvram Deamon read P sensor calibration data FAIL\n");
            NVM_CloseFileDesc(p_sensor_threshold_fd);
        }

    } else {
        ALOGE("********PL Nvram Struc malloc Fai or Nvram_is not Readyl!!*********\n");
        printf("********PL Nvram Struc malloc Fai or Nvram_is not Readyl!!*********\n");
    }

    free(p_sensor_threshold);

    return nvram_result;
}

int write_back_to_driver(STK3X1X_THRESHOLD *stk3x1x_threshold)
{
    char buffer[100];
    int fd = -1;
    int err = 0, res = 0;

    strcpy(input_sysfs_path, "/sys/alsps/link/cali");

    ALOGI("[STK]=============write PL sensor data to driver start=============\n");
    printf("[STK]=============write PL sensor data to driver start=============\n");
    if (stk3x1x_threshold->ps_high_thd == 0 ||
            stk3x1x_threshold->ps_low_thd == 0 ||
            stk3x1x_threshold->result_ct == 0 ) {

        ALOGE("[STK]There are no PL sensor calibration data =0 !\n");
        ALOGE("[STK]Show P -Sensor value => %d, %d, %d %d\n",
                stk3x1x_threshold->ps_high_thd,
                stk3x1x_threshold->ps_low_thd,
                stk3x1x_threshold->result_ct,
                stk3x1x_threshold->result_ps_cover);

        printf("[STK]There are no PL sensor calibration data =0 !\n");
        printf("[STK]Show P -Sensor value => %d, %d, %d %d\n",
                stk3x1x_threshold->ps_high_thd,
                stk3x1x_threshold->ps_low_thd,
                stk3x1x_threshold->result_ct,
                stk3x1x_threshold->result_ps_cover);

        return -1;
    }

    ALOGI("%s:[STK]input_sysfs_path = %s\n", __func__, input_sysfs_path);
    printf("%s:[STK]input_sysfs_path = %s\n", __func__, input_sysfs_path);
    fd = open(input_sysfs_path, O_RDWR);

    if (fd >= 0) {
        int bytes = sprintf(buffer, "%x %x %x %x %x %x %x %x %x\n",
                stk3x1x_threshold->ps_high_thd,
                stk3x1x_threshold->ps_low_thd,
                stk3x1x_threshold->result_ct,
                stk3x1x_threshold->result_ps_cover,
                stk3x1x_threshold->result_als_value,
                stk3x1x_threshold->cci_als_adc_test,
                stk3x1x_threshold->cci_als_lux_cali,
                stk3x1x_threshold->cci_transmittance_cali,
                stk3x1x_threshold->cci_als_adc_cali);

        ALOGI("[STK]Write P -Sensor value to driver %d, %d, %d, %d bytes = %d\n",
                stk3x1x_threshold->ps_high_thd,
                stk3x1x_threshold->ps_low_thd,
                stk3x1x_threshold->result_ct,
                stk3x1x_threshold->result_ps_cover,
                bytes);
        ALOGI("[STK]Wriet L -Sensor value to driver %d, %d, %d, %d, %d\n",
                stk3x1x_threshold->result_als_value,
                stk3x1x_threshold->cci_als_adc_test,
                stk3x1x_threshold->cci_als_lux_cali,
                stk3x1x_threshold->cci_transmittance_cali,
                stk3x1x_threshold->cci_als_adc_cali);

        printf("[STK]Write P -Sensor value to driver %d, %d, %d, %d bytes = %d\n",
                stk3x1x_threshold->ps_high_thd,
                stk3x1x_threshold->ps_low_thd,
                stk3x1x_threshold->result_ct,
                stk3x1x_threshold->result_ps_cover,
                bytes);
        printf("[STK]Wriet L -Sensor value to driver %d, %d, %d, %d, %d\n",
                stk3x1x_threshold->result_als_value,
                stk3x1x_threshold->cci_als_adc_test,
                stk3x1x_threshold->cci_als_lux_cali,
                stk3x1x_threshold->cci_transmittance_cali,
                stk3x1x_threshold->cci_als_adc_cali);

        int res = write(fd, buffer, bytes);
        if (res >= 0) {
            ALOGI("[STK]Write cali data to %s Success!\n", input_sysfs_path);
            printf("[STK]Write cali data to %s Success!\n", input_sysfs_path);
        } else {
            /* Retry - write cali data to kernel */
            ALOGE("[STK]Write cali data to %s Fail res = %d!\n", input_sysfs_path, res);
            printf("[STK]Write cali data to %s Fail res = %d!\n", input_sysfs_path, res);
            close(fd);

            return -1;
        }
    } else {
        ALOGE("[STK]Open %s Fail!!\n", input_sysfs_path);
        printf("[STK]Open %s Fail!!\n", input_sysfs_path);

        return -1;
    }
    close(fd);
    ALOGI("[STK]=============write PL sensor data to driver end=============\n");
    printf("[STK]=============write PL sensor data to driver end=============\n");

    return 0;
}