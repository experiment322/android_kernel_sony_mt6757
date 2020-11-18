/*
 * Copyright (C) 2014 Sony Mobile Communications AB.
 * All rights, including trade secret rights, reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <fcntl.h>
#include "Custom_NvRam_LID.h"
#include "libnvram.h"
/* CCI SW Version */
#include "CFG_SW_VER_File.h"
#include "CFG_SW_VER_Default.h"
#include "CFG_PROJ_ID_File.h"
#include "CFG_PROJ_ID_Default.h"
/* CCI SW Version */
#include <cutils/properties.h>
#include <android/log.h>
#include <string.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , "CCINVRAM",__VA_ARGS__)
#define TAG "CCINVRAM"


/* In kernel/drivers/misc/mediatek/cei_hw_id/cei_hw_id.h
enum cei_project_type {        
                               CEI_PROJECT_BY57 = 0, // CAT6 GINA SS
                               CEI_PROJECT_BY58 = 1, // CAT6 GINA APAC SS
                               CEI_PROJECT_BY59 = 2, // CAT6 REX SS    
                               CEI_PROJECT_BY66 = 8, // CAT4 GINA SS
                               CEI_PROJECT_BY67 = 12, // CAT4 GINA DS
                               CEI_PROJECT_BY68 = 13, // CAT4 GINA APAC DS     
                               CEI_PROJECT_BY69 = 10 // CAT4 REX SS   
                        };
*/
#define CEI_PROJECTID_MAX 13
const char cei_project_str[][16] =
{
        "BY57",  	  //ro.cei.projectid = 0
        "BY58",     //ro.cei.projectid = 1
        "BY59",     //ro.cei.projectid = 2
        "unknown",  //ro.cei.projectid = 3
        "unknown",  //ro.cei.projectid = 4
        "unknown",  //ro.cei.projectid = 5
        "unknown",  //ro.cei.projectid = 6
        "unknown",  //ro.cei.projectid = 7
        "BY66",     //ro.cei.projectid = 8
        "unknown",  //ro.cei.projectid = 9
        "BY69",     //ro.cei.projectid = 10
        "unknown",  //ro.cei.projectid = 11
        "BY67",     //ro.cei.projectid = 12
        "BY68",     //ro.cei.projectid = 13
};

const char cei_project_str_redwood[][16] =
{
        "BY86",  	  //ro.cei.projectid = 0
        "unknown",     //ro.cei.projectid = 1
        "BY88",     //ro.cei.projectid = 2
        "BY78",  //ro.cei.projectid = 3
        "unknown",  //ro.cei.projectid = 4
        "BY87",  //ro.cei.projectid = 5
        "unknown",  //ro.cei.projectid = 6
        "unknown",  //ro.cei.projectid = 7
        "unknown",     //ro.cei.projectid = 8
        "BY77",  //ro.cei.projectid = 9
        "BY78",     //ro.cei.projectid = 10
        "unknown",  //ro.cei.projectid = 11
        "BY76",     //ro.cei.projectid = 12
        "unknown",     //ro.cei.projectid = 13
};

const char cei_project_str_teak[][16] =
{
        "BY61",  	  //ro.cei.projectid = 0
        "unknown",     //ro.cei.projectid = 1
        "BY65",     //ro.cei.projectid = 2
        "unknown",  //ro.cei.projectid = 3
        "unknown",  //ro.cei.projectid = 4
        "BY63",  //ro.cei.projectid = 5
        "unknown",  //ro.cei.projectid = 6
        "unknown",  //ro.cei.projectid = 7
        "unknown",     //ro.cei.projectid = 8
        "unknown",  //ro.cei.projectid = 9
        "unknown",     //ro.cei.projectid = 10
        "unknown",  //ro.cei.projectid = 11
        "BY62",     //ro.cei.projectid = 12
        "BY64",     //ro.cei.projectid = 13
};

#define STRING_LEN_UNDEFINED 9

int main()
{
	int rec_size = 0;
	int	rec_num = 0;
	int	result = 0;

	F_ID nvram_fd;
	FILE_SW_VER_STRUCT *myccisw = NULL;
	FILE_PROJ_ID_STRUCT *mycciproj = NULL;	
	int file_cci_lid = AP_CFG_CUSTEOM_FILE_SW_VER_LID;
	char tmp[PROPERTY_VALUE_MAX] = {0};	
	char nvram_init_val[PROPERTY_VALUE_MAX] = {0};
	unsigned long proj_id = 0;
	char tmp_hwid[PROPERTY_VALUE_MAX] = {0};
	int result_hwid = 0;
	unsigned long hw_id = 0;

	LOGI( "=========== CCINVRAM write: Enter ===========\n");

	while(1)
	{
		property_get("service.nvram_init",nvram_init_val,"WAITING");
		LOGI("===========Nvram Deamon status: %s===========\n",nvram_init_val);

		if((strcmp(nvram_init_val,"Ready")==0) || (strcmp(nvram_init_val,"Pre_Ready")==0))
		{
			break;
		}
		else
		{
			usleep(500*1000);
		}
	}


	/* CCI SW Version */

	myccisw = (FILE_SW_VER_STRUCT *) malloc(sizeof(FILE_SW_VER_STRUCT));
	if (myccisw != NULL) {
		memset(myccisw , 0, sizeof(FILE_SW_VER_STRUCT));
		
		nvram_fd = NVM_GetFileDesc(file_cci_lid, &rec_size, &rec_num, ISREAD);

		if (nvram_fd.iFileDesc >= 0) {
			result = read(nvram_fd.iFileDesc, myccisw, rec_size*rec_num); 
			if (result > 0) {
				result = property_get("ro.build.display.id", tmp, "undefined");
				if (result > 0) {
					LOGI("tmp(ro.build.display.id) = %s, myccisw->sw_ver = %s\n", tmp, myccisw->sw_ver);
					
					if (strcmp(tmp, myccisw->sw_ver)) {
		    				memcpy(myccisw->sw_ver, tmp, sizeof(char)*64);
						myccisw->sw_ver[63] = '\0';
							
						LOGI(" write SW_VER: mycciproj->sw_ver=%s\n", myccisw->sw_ver);
						
			    			nvram_fd = NVM_GetFileDesc(file_cci_lid, &rec_size, &rec_num, ISWRITE);
			    			result = write(nvram_fd.iFileDesc, myccisw, rec_size*rec_num); 
		    				if (result < 0) {
		        					LOGI("Fail to write NVRAM: SW_VER\n");
		    				}
					}
				} else {
					LOGI("Fail to get property: ro.build.display.id\n");
				}
			} else {
				LOGI("Fail to read NVRAM: SW_VER\n");
			}
			NVM_CloseFileDesc(nvram_fd);			
		}else {
			LOGI("Fail to NVM_GetFileDesc: SW_VER\n");
		}
		free(myccisw);
	}else {
	
		printf("Fail to malloc NVRAM: FILE_SW_VER_STRUCT\n");
	}


	/* CCI SW Version */
	
	//=================================================//
	/* CCI PROJECT ID */
	
	file_cci_lid = AP_CFG_CUSTEOM_FILE_PROJ_ID_LID;
	memset( tmp, 0, PROPERTY_VALUE_MAX);

	mycciproj = (FILE_PROJ_ID_STRUCT *) malloc(sizeof(FILE_PROJ_ID_STRUCT));
	if (mycciproj != NULL) {
		memset(mycciproj, 0, sizeof(FILE_PROJ_ID_STRUCT));

		nvram_fd = NVM_GetFileDesc(file_cci_lid, &rec_size, &rec_num, ISREAD);

		if (nvram_fd.iFileDesc >= 0) {
		
			result = read(nvram_fd.iFileDesc, mycciproj, rec_size*rec_num); 
			
			if (result > 0) {
				result = property_get("ro.cei.projectid", tmp, "undefined");
				if (result > 0) {
					if(strncmp(tmp, "undefined", STRING_LEN_UNDEFINED) == 0) {
						memcpy(mycciproj->proj_id, "undefined", STRING_LEN_UNDEFINED);
						mycciproj->proj_id[STRING_LEN_UNDEFINED] = '\0';
						
						LOGI(" write PROJ_ID: mycciproj->proj_id=%s\n", mycciproj->proj_id);
						
						nvram_fd = NVM_GetFileDesc(file_cci_lid, &rec_size, &rec_num, ISWRITE);
						result = write(nvram_fd.iFileDesc, mycciproj, rec_size*rec_num);
						if (result < 0) {
						LOGI("Fail to write NVRAM: PROJ_ID\n");
						}
				  }
				  else {
	        	proj_id = strtoul(tmp, NULL, 10);
	        	LOGI("proj_id = %lu, ro.cei.projectid = %s\n", proj_id, tmp);
	        	
	        	if(proj_id <= (unsigned long)CEI_PROJECTID_MAX)
						{
							memset( tmp_hwid, 0, PROPERTY_VALUE_MAX);
							result_hwid = property_get("ro.cei.customer_projectid", tmp_hwid, "undefined");
							hw_id = strtoul(tmp_hwid, NULL, 10);

							if(hw_id==1)
							{
								LOGI("cei_project_str_redwood[%lu]=%s, mycciproj->proj_id=%s\n", proj_id, cei_project_str_redwood[proj_id], mycciproj->proj_id);

								if (strncmp(cei_project_str_redwood[proj_id], mycciproj->proj_id, 16)) {
									memcpy(mycciproj->proj_id, cei_project_str_redwood[proj_id], sizeof(char)*16);
									mycciproj->proj_id[15] = '\0';

									LOGI(" write PROJ_ID: mycciproj->proj_id=%s\n", mycciproj->proj_id);
	
									nvram_fd = NVM_GetFileDesc(file_cci_lid, &rec_size, &rec_num, ISWRITE);
									result = write(nvram_fd.iFileDesc, mycciproj, rec_size*rec_num);
									if (result < 0) {
										LOGI("Fail to write NVRAM: PROJ_ID\n");
									}
								}
							}
							else if(hw_id==3)
							{
								LOGI("cei_project_str_teak[%lu]=%s, mycciproj->proj_id=%s\n", proj_id, cei_project_str_teak[proj_id], mycciproj->proj_id);

								if (strncmp(cei_project_str_teak[proj_id], mycciproj->proj_id, 16)) {
									memcpy(mycciproj->proj_id, cei_project_str_teak[proj_id], sizeof(char)*16);
									mycciproj->proj_id[15] = '\0';

									LOGI(" write PROJ_ID: mycciproj->proj_id=%s\n", mycciproj->proj_id);
	
									nvram_fd = NVM_GetFileDesc(file_cci_lid, &rec_size, &rec_num, ISWRITE);
									result = write(nvram_fd.iFileDesc, mycciproj, rec_size*rec_num);
									if (result < 0) {
										LOGI("Fail to write NVRAM: PROJ_ID\n");
									}
								}
							}
							else
							{
								LOGI("cei_project_str[%lu]=%s, mycciproj->proj_id=%s\n", proj_id, cei_project_str[proj_id], mycciproj->proj_id);

								if (strncmp(cei_project_str[proj_id], mycciproj->proj_id, 16)) {
									memcpy(mycciproj->proj_id, cei_project_str[proj_id], sizeof(char)*16);
									mycciproj->proj_id[15] = '\0';

									LOGI(" write PROJ_ID: mycciproj->proj_id=%s\n", mycciproj->proj_id);
	
									nvram_fd = NVM_GetFileDesc(file_cci_lid, &rec_size, &rec_num, ISWRITE);
									result = write(nvram_fd.iFileDesc, mycciproj, rec_size*rec_num);
									if (result < 0) {
										LOGI("Fail to write NVRAM: PROJ_ID\n");
									}
								}
							}
						}
						else {
							LOGI("Fail : ro.cei.projectid is out of range\n");
					  }
	        }
					

				} else {
					LOGI("Fail to get property: ro.cei.projectid\n");
				}
			} else {
				LOGI("Fail to read NVRAM: PROJ_ID\n");
			}

			NVM_CloseFileDesc(nvram_fd);
		}else{
			printf("Fail toNVM_GetFileDesc: PROJ_ID\n");			
		}
		free(mycciproj);
	}else {
	
		LOGI("Fail to malloc NVRAM: FILE_PROJ_ID_STRUCT\n");
	}
	/* CCI PROJECT ID */

    return 0;
}

