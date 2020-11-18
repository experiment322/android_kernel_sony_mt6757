#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cutils/properties.h>
#include <private/android_filesystem_config.h>

#define LOG_NIDEBUG 0
#define LOG_TAG "klogcat"
#include <utils/Log.h>

#include "libklog.h"
#include "cciklog.h"

static struct KSMS ksms;
static int ClearFlag = 0;

int check_old_log(void)
{
	int flag = 0;

	perform_klog_ioctl(KLOG_IOCTL_CHECK_OLD_LOG, &flag);

	return flag;
}

int clear_klog(int force)
{
	if(ClearFlag == 0)//prevent mutiple clear in one entrance
	{
		ClearFlag = 1;
		return perform_klog_ioctl(KLOG_IOCTL_CLEAR_LOG, &force);
	}
	else
	{
		return KLOG_ERROR;
	}
}

unsigned int get_driver_version(void)
{
	unsigned int version = 0;

	perform_klog_ioctl(KLOG_IOCTL_GET_KLOG_VERSION, &version);

	return version;
}

//Get current time and date
int get_time(char *timestamp, int size)
{
	char format[] = "%Y%m%d%H%M%S";
	time_t timep;
	struct tm *p;

	time(&timep);
	p = localtime(&timep);
	strftime(timestamp, size, format, p);

//Retrun seconds
	return (int)timep;
}

#ifdef CCI_KLOG_ALLOW_FORCE_PANIC
int klogcat_panic(void)
{
	return perform_klog_ioctl(KLOG_IOCTL_FORCE_PANIC, NULL);
}

int klogcat_crash(void)
{
	return perform_klog_ioctl(KLOG_IOCTL_FORCE_CRASH, NULL);
}

int klogcat_panic_when_suspend(int enable)
{
	return perform_klog_ioctl(KLOG_IOCTL_SET_PANIC_WHEN_SUSPEND, &enable);
}

int klogcat_panic_when_power_off(int enable)
{
	return perform_klog_ioctl(KLOG_IOCTL_SET_PANIC_WHEN_POWER_OFF, &enable);
}

int klogcat_set_magic(const char* magic)
{
	return perform_klog_ioctl(KLOG_IOCTL_SET_MAGIC, magic);
}
#endif // #ifdef CCI_KLOG_ALLOW_FORCE_PANIC

#ifdef CCI_KLOG_SUPPORT_RESTORATION
int klogcat_restore_klog(void)
{
	FILE *fp = NULL;
	struct system_information sysinfo;
	struct system_information tempsysinfo;//temporary klog file
	long length = 0;
	char *klog_content = NULL;
	int ret = KLOG_OK;

	fp = fopen(KLOG_TEMP_FILE, "rb");
	if(fp)
	{
		fseek(fp ,0, SEEK_END);
		length = ftell(fp);
		if(length == CCI_KLOG_SIZE)
		{
			fseek(fp ,0, SEEK_SET);
			if(fread(&tempsysinfo, sizeof(struct system_information), 1, fp) == 0)
			{
				ALOGE("read temp klog header failed");
				printf("read temp klog header failed");
				ret = KLOG_ERROR;
			}
			else
			{
//check klog header
				if(strcmp(tempsysinfo.klog_signature, KLOG_SIGNATURE) == 0 && tempsysinfo.klog_header_version == KLOG_HEADER_VERSION && tempsysinfo.klog_version == get_driver_version())
				{
//check image versions
					prepare_sysinfo(&sysinfo);
/*
					ALOGI("current modem=%s, tempklog modem=%s", sysinfo.modem_version, tempsysinfo.modem_version);
					ALOGI("current linux=%s, tempklog linux=%s", sysinfo.linux_version, tempsysinfo.linux_version);
					ALOGI("current android=%s, tempklog android=%s", sysinfo.android_version, tempsysinfo.android_version);
					ALOGI("current build=%s, tempklog build=%s", sysinfo.build_version, tempsysinfo.build_version);
					ALOGI("current flex=%s, tempklog flex=%s", sysinfo.flex_version, tempsysinfo.flex_version);
					ALOGI("current version_id=%s, tempklog version_id=%s", sysinfo.version_id, tempsysinfo.version_id);
					ALOGI("current date=%s, tempklog date=%s", sysinfo.build_date, tempsysinfo.build_date);
					ALOGI("current type=%s, tempklog type=%s", sysinfo.build_type, tempsysinfo.build_type);
					ALOGI("current user=%s, tempklog user=%s", sysinfo.build_user, tempsysinfo.build_user);
					ALOGI("current host=%s, tempklog host=%s", sysinfo.build_host, tempsysinfo.build_host);
					ALOGI("current key=%s, tempklog key=%s", sysinfo.build_key, tempsysinfo.build_key);
					ALOGI("current model=%s, tempklog model=%s", sysinfo.product_model, tempsysinfo.product_model);
*/
					if(
						(strcmp(sysinfo.modem_version, "-") == 0 || strcmp(tempsysinfo.modem_version, sysinfo.modem_version) == 0)
						&& strcmp(tempsysinfo.linux_version, sysinfo.linux_version) == 0
						&& strcmp(tempsysinfo.android_version, sysinfo.android_version) == 0
						&& strcmp(tempsysinfo.build_version, sysinfo.build_version) == 0
						&& strcmp(tempsysinfo.flex_version, sysinfo.flex_version) == 0
						&& strcmp(tempsysinfo.version_id, sysinfo.version_id) == 0
						&& strcmp(tempsysinfo.build_date, sysinfo.build_date) == 0
						&& strcmp(tempsysinfo.build_type, sysinfo.build_type) == 0
						&& strcmp(tempsysinfo.build_user, sysinfo.build_user) == 0
						&& strcmp(tempsysinfo.build_host, sysinfo.build_host) == 0
						&& strcmp(tempsysinfo.build_key, sysinfo.build_key) == 0
						&& strcmp(tempsysinfo.product_model, sysinfo.product_model) == 0
					  )
					{
						klog_content = malloc(CCI_KLOG_SIZE);
						if(klog_content)
						{
							fseek(fp ,0, SEEK_SET);
							if(fread(klog_content, CCI_KLOG_SIZE, 1, fp) == 0)
							{
								ALOGE("read temp klog content failed");
								printf("read temp klog content failed");
								ret = KLOG_ERROR;
							}
							else
							{
								perform_klog_ioctl(KLOG_IOCTL_RESTORE_LOG, klog_content);
							}
							free(klog_content);
						}
						else
						{
							ALOGE("out of memory for restoring temp klog");
							printf("out of memory for restoring temp klog");
							ret = KLOG_ERROR;
						}
					}
					else
					{
						ALOGE("temp klog images mismatch");
						printf("temp klog images mismatch");
						ret = KLOG_ERROR;
					}
				}
				else
				{
					ALOGE("temp klog header mismatch");
					printf("temp klog header mismatch");
					ret = KLOG_ERROR;
				}
			}
		}
		else
		{
			ALOGE("temp klog size mismatch");
			printf("temp klog size mismatch");
			ret = KLOG_ERROR;
		}
		fclose(fp);
		unlink(KLOG_TEMP_FILE);
	}
	else
	{
		ALOGE("open temporary klog file failed");
		printf("open temporary klog file failed");
		ret = KLOG_ERROR;
	}

	return ret;
}
#endif // #ifdef CCI_KLOG_SUPPORT_RESTORATION

int main(int argc, char **argv)
{
	char klogfile[256] = {0};
	char cmdline[256] = {0};
	char pname[256] = {0};
	char fname[256] = {0};
	char timestamp[32] = {0};
	char caller[1024] = {0};
	char klog_max[PROPERTY_VALUE_MAX] = {0};
	char klog_userdata_count[PROPERTY_VALUE_MAX] = {0};
	char klog_internal_count[PROPERTY_VALUE_MAX] = {0};
	char klog_external_count[PROPERTY_VALUE_MAX] = {0};
	char boot_completed[PROPERTY_VALUE_MAX] = {0};
	int AllowCustomFilename = 0;
	int AllowCustomPath = 0;
	int InternalSaved = 0;
	int ExternalSaved = 0;
	int CrashFlag = 0;
	int BootCompletedFlag = 0;
	int dump_flag = 0;
	int dump_retry = 0;
	int ret = KLOG_OK;
	int rc = KLOG_OK;
	int i = 0;
	pid_t klogcat_ppid;
	uid_t klogcat_puid;
#ifdef KLOG_DEBUG_REFERENCE_PROPERTY
	char enter_dl_mode[PROPERTY_VALUE_MAX] = {0};
	int KlogDumpMode = 0;

	property_get("ro.enter_dl_mode", enter_dl_mode, "0");
	if(strcmp(enter_dl_mode, "1") == 0)
	{
		KlogDumpMode = 1;
	}
#else // #ifdef KLOG_DEBUG_REFERENCE_PROPERTY
	int KlogDumpMode = 1;
#endif // #ifdef KLOG_DEBUG_REFERENCE_PROPERTY

	klogcat_ppid = getppid();
	get_pid_name(klogcat_ppid, &caller);

	if(argc > 1)
	{
		get_time(timestamp, sizeof(timestamp));

		if(KlogDumpMode)
		{
			ksms = checkStorageMountStatus();
		}

		if(strcmp(argv[1], KLOG_COMMAND_RECORD_SYSINFO) == 0)
		{
			record_sysinfo(1, 2);
			ALOGI("Update sysinfo");
		}
		else if(strcmp(argv[1], KLOG_COMMAND_CHECK_KLOG) == 0 || strcmp(argv[1], KLOG_COMMAND_KLOG_INIT) == 0)
		{
			CrashFlag = check_old_log();
#ifdef CCI_KLOG_SUPPORT_RESTORATION
//workaround for DDR reset issue
			if(CrashFlag == KLOG_INDEX_INVALID || CrashFlag == KLOG_INDEX_INIT)
			{
				if(klogcat_restore_klog() != KLOG_ERROR)
				{
					CrashFlag = check_old_log();//restore then check again
				}
			}
			unlink(KLOG_TEMP_FILE);
#endif // #ifdef CCI_KLOG_SUPPORT_RESTORATION

//set crashflag property
			if(CrashFlag == KLOG_INDEX_MARM_FATAL || CrashFlag == KLOG_INDEX_AARM_PANIC || CrashFlag == KLOG_INDEX_RPM_CRASH || CrashFlag == KLOG_INDEX_SUBSYS_CRASH || CrashFlag == KLOG_INDEX_FIQ_HANG || CrashFlag == KLOG_INDEX_UNKNOWN_CRASH)
			{
				snprintf(cmdline, sizeof(cmdline), "0x%X", atoi(timestamp));
				property_set("sys.crashid", cmdline);
				snprintf(cmdline, sizeof(cmdline), "%d", CrashFlag);
				property_set("sys.crashflag", cmdline);

//get magic index
				for(i = 0; i < sizeof(kml) / sizeof(struct klog_magic_list); i++)
				{
					if(CrashFlag == kml[i].index)
					{
						CrashFlag = i;
						break;
					}
				}

//dump klog
				if(KlogDumpMode)
				{
//get count settings
					property_get(KLOG_PROPERTY_NAME_MAX, klog_max, KLOG_PROPERTY_VALUE_MAX);
					if(atoi(klog_max) > KLOG_FILES_MAX)
					{
						snprintf(klog_max, sizeof(klog_max), "%d", KLOG_FILES_MAX);
						property_set(KLOG_PROPERTY_NAME_MAX, klog_max);
					}

					property_get(KLOG_PROPERTY_NAME_USERDATA_COUNT, klog_userdata_count, KLOG_PROPERTY_VALUE_USERDATA_COUNT);
					snprintf(klog_userdata_count, sizeof(klog_userdata_count), "%d", (atoi(klog_userdata_count) + 1) % atoi(klog_max));

//USERDATA
#ifdef KLOG_PATH_USERDATA
					if(ksms.UserdataMounted == 1)
					{
						snprintf(cmdline, sizeof(cmdline), "/system/bin/mkdir -p %s", KLOG_PATH_USERDATA);
						system(cmdline);

						snprintf(cmdline, sizeof(cmdline), "/system/bin/rm %s/%s%s-*", KLOG_PATH_USERDATA, KLOG_FILENAME, klog_userdata_count);
						system(cmdline);
						ALOGI("Remove old KLog files %s from USERDATA", cmdline);

						snprintf(cmdline, sizeof(cmdline), "%s/%s%s-%s-%s%s", KLOG_PATH_USERDATA, KLOG_FILENAME, klog_userdata_count, kml[CrashFlag].name, timestamp, KLOG_FILENAME_EXT);
						if(strlen(klogfile))
						{
							strncat(cmdline, KLOG_FILENAME_EXT_GZIP, strlen(KLOG_FILENAME_EXT_GZIP));
							ret = copy_file(klogfile, cmdline);
						}
						else
						{
							ret = dump_to_target(cmdline, kml[CrashFlag].index);
						}
						if(ret == KLOG_OK)
						{
							if(strlen(klogfile) == 0)
							{
								strncpy(klogfile, cmdline, strlen(cmdline));
								strncat(klogfile, KLOG_FILENAME_EXT_GZIP, strlen(KLOG_FILENAME_EXT_GZIP));
								dump_flag = 1;
							}
							ALOGI("Write %s to USERDATA succeed", cmdline);
							property_set(KLOG_PROPERTY_NAME_USERDATA_COUNT, klog_userdata_count);
						}
						else
						{
							ALOGE("Write %s to USERDATA failed", cmdline);
							rc = KLOG_ERROR;
						}
					}
#endif // #ifdef KLOG_PATH_USERDATA

//internal and external storage
					dump_retry = 0;
					do
					{
						if(dump_retry)
						{
							sleep(1);//Sleep 1 second for waiting SD card ready
//check boot_completed or not
							if(BootCompletedFlag == 0)
							{
								property_get("sys.boot_completed", boot_completed, "0");
								if(strcmp(boot_completed, "1") == 0)
								{
									BootCompletedFlag = 1;
									dump_retry = 0;//reset klog init dump retry counter
								}
							}
						}
						ksms = checkStorageMountStatus();

						property_get(KLOG_PROPERTY_NAME_INTERNAL_COUNT, klog_internal_count, KLOG_PROPERTY_VALUE_INTERNAL_COUNT);
						snprintf(klog_internal_count, sizeof(klog_internal_count), "%d", (atoi(klog_internal_count) + 1) % atoi(klog_max));

						property_get(KLOG_PROPERTY_NAME_EXTERNAL_COUNT, klog_external_count, KLOG_PROPERTY_VALUE_EXTERNAL_COUNT);
						snprintf(klog_external_count, sizeof(klog_external_count), "%d", (atoi(klog_external_count) + 1) % atoi(klog_max));

#ifdef KLOG_PATH_INTERNAL
						if(InternalSaved == 0 && ksms.InternalMounted == 1)
						{
							InternalSaved = 1;

							snprintf(cmdline, sizeof(cmdline), "/system/bin/mkdir -p %s", KLOG_PATH_INTERNAL);
							system(cmdline);

							snprintf(cmdline, sizeof(cmdline), "/system/bin/rm %s/%s%s-*", KLOG_PATH_INTERNAL, KLOG_FILENAME, klog_internal_count);
							system(cmdline);
							ALOGI("Remove old KLog files %s from internal SD", cmdline);

							snprintf(cmdline, sizeof(cmdline), "%s/%s%s-%s-%s%s", KLOG_PATH_INTERNAL, KLOG_FILENAME, klog_internal_count, kml[CrashFlag].name, timestamp, KLOG_FILENAME_EXT);
							if(strlen(klogfile))
							{
								strncat(cmdline, KLOG_FILENAME_EXT_GZIP, strlen(KLOG_FILENAME_EXT_GZIP));
								ret = copy_file(klogfile, cmdline);
							}
							else
							{
								ret = dump_to_target(cmdline, kml[CrashFlag].index);
							}
							if(ret == KLOG_OK)
							{
								if(strlen(klogfile) == 0)
								{
									strncpy(klogfile, cmdline, strlen(cmdline));
									strncat(klogfile, KLOG_FILENAME_EXT_GZIP, strlen(KLOG_FILENAME_EXT_GZIP));
									dump_flag = 1;
								}
								ALOGI("Write %s to internal SD succeed", cmdline);
								property_set(KLOG_PROPERTY_NAME_INTERNAL_COUNT, klog_internal_count);
							}
							else
							{
								ALOGE("Write %s to internal SD failed", cmdline);
								rc = KLOG_ERROR;
							}
						}
#endif // #ifdef KLOG_PATH_INTERNAL
#ifdef KLOG_PATH_EXTERNAL
						if(ExternalSaved == 0 && ksms.ExternalMounted == 1)
						{
							ExternalSaved = 1;

							snprintf(cmdline, sizeof(cmdline), "/system/bin/mkdir -p %s", KLOG_PATH_EXTERNAL);
							system(cmdline);

							snprintf(cmdline, sizeof(cmdline), "/system/bin/rm %s/%s%s-*", KLOG_PATH_EXTERNAL, KLOG_FILENAME, klog_external_count);
							system(cmdline);
							ALOGI("Remove old KLog files %s from external SD", cmdline);

							snprintf(cmdline, sizeof(cmdline), "%s/%s%s-%s-%s%s", KLOG_PATH_EXTERNAL, KLOG_FILENAME, klog_external_count, kml[CrashFlag].name, timestamp, KLOG_FILENAME_EXT);
							if(strlen(klogfile))
							{
								strncat(cmdline, KLOG_FILENAME_EXT_GZIP, strlen(KLOG_FILENAME_EXT_GZIP));
								ret = copy_file(klogfile, cmdline);
							}
							else
							{
								ret = dump_to_target(cmdline, kml[CrashFlag].index);
							}
							if(ret == KLOG_OK)
							{
								if(strlen(klogfile) == 0)
								{
									strncpy(klogfile, cmdline, strlen(cmdline));
									strncat(klogfile, KLOG_FILENAME_EXT_GZIP, strlen(KLOG_FILENAME_EXT_GZIP));
									dump_flag = 1;
								}
								ALOGI("Write %s to external SD succeed", cmdline);
								property_set(KLOG_PROPERTY_NAME_EXTERNAL_COUNT, klog_external_count);
							}
							else
							{
								ALOGE("Write %s to external SD failed", cmdline);
								rc = KLOG_ERROR;
							}
						}
#endif // #ifdef KLOG_PATH_EXTERNAL
						dump_retry++;
					}
					while(((InternalSaved == 0 && ksms.InternalMounted == 0) || (ExternalSaved == 0 && ksms.ExternalMounted == 0)) && ((BootCompletedFlag == 0 && dump_retry < WAIT_INIT_DUMP_TIME_NOT_BOOTED) || (BootCompletedFlag == 1 && dump_retry < WAIT_INIT_DUMP_TIME_BOOT_COMPLETED)));//retry
				}

//clear
				clear_klog(dump_flag);
				ALOGI("Clear KLOG and restart logging");
			}
			else
			{
				property_set("sys.crashflag", "-");
				ALOGI("No crash log");
			}

			if(strcmp(argv[1], KLOG_COMMAND_KLOG_INIT) == 0)
			{
				record_sysinfo(1, 0);
				ALOGI("Init sysinfo");
			}
		}
		else if(strcmp(argv[1], KLOG_COMMAND_RECORD_BOOT_COMPLETE) == 0)//boot completed
		{
			record_sysinfo(1, 1);
			ALOGI("Boot complete");
		}
		else if(strcmp(argv[1], KLOG_COMMAND_RECORD_USER_PWROFF) == 0)//user confirmed to power-off
		{
			record_sysinfo(0, 1);
			ALOGI("Power-off Android start");
		}
		else if(strcmp(argv[1], KLOG_COMMAND_RECORD_KERNEL_PWROFF) == 0)//perform low-level power-off
		{
			record_sysinfo(0, 2);
			ALOGI("Power-off Android end");
		}
		else if(strcmp(argv[1], KLOG_COMMAND_CLEAR_KLOG_BUFFER) == 0)
		{
			clear_klog(0);
			ALOGI("Clear KLOG and restart logging except crashing");
		}

		if(KlogDumpMode)
		{
			if(strcmp(argv[1], KLOG_COMMAND_DUMP_KLOG) == 0)
			{
//check if there is a valid customized file name
				if(argv[2] && strlen(argv[2]) > 0)
				{
					AllowCustomFilename = 1;
//check customized path
					if(argv[2][0] == '/')
					{
//workaround for dumping klogcat command from LogFilesDumper
						klogcat_puid = getuid();
						ALOGI("UID:%d is trying to dump klog", klogcat_puid);
						if(klogcat_puid >= AID_APP || klogcat_puid == AID_SYSTEM)//Android apps || system
						{
#ifdef KLOG_PATH_USERDATA
							if(AllowCustomPath == 0 && ksms.UserdataMounted == 1
								&& (strncmp(argv[2], KLOG_PATH_USERDATA_PARTITION"/klog", strlen(KLOG_PATH_USERDATA_PARTITION"/klog")) == 0
								|| strncmp(argv[2], KLOG_PATH_USERDATA_PARTITION"/crash", strlen(KLOG_PATH_USERDATA_PARTITION"/crash")) == 0
								)
							)
							{
								AllowCustomPath = 1;
								ksms.ExternalMounted = 0;
								ksms.InternalMounted = 0;
							}
#endif // #ifdef KLOG_PATH_USERDATA
						}
#ifdef KLOG_PATH_EXTERNAL
						if(AllowCustomPath == 0 && ksms.ExternalMounted == 1
							&& (strncmp(argv[2], KLOG_PATH_EXTERNAL_PARTITION"/", strlen(KLOG_PATH_EXTERNAL_PARTITION"/")) == 0
#ifdef KLOG_PATH_EXTERNAL_PARTITION_ALIAS
							|| strncmp(argv[2], KLOG_PATH_EXTERNAL_PARTITION_ALIAS"/", strlen(KLOG_PATH_EXTERNAL_PARTITION_ALIAS"/")) == 0
#endif // #ifdef KLOG_PATH_EXTERNAL_PARTITION_ALIAS
							)
						)
						{
							AllowCustomPath = 1;
							ksms.InternalMounted = 0;
							ksms.UserdataMounted = 0;
						}
#endif // #ifdef KLOG_PATH_EXTERNAL
#ifdef KLOG_PATH_INTERNAL
						if(AllowCustomPath == 0 && ksms.InternalMounted == 1
							&& (strncmp(argv[2], KLOG_PATH_INTERNAL_PARTITION"/", strlen(KLOG_PATH_INTERNAL_PARTITION"/")) == 0
#ifdef KLOG_PATH_INTERNAL_PARTITION_ALIAS
							|| strncmp(argv[2], KLOG_PATH_INTERNAL_PARTITION_ALIAS"/", strlen(KLOG_PATH_INTERNAL_PARTITION_ALIAS"/")) == 0)
#endif // #ifdef KLOG_PATH_INTERNAL_PARTITION_ALIAS
							)
						{
							AllowCustomPath = 1;
							ksms.ExternalMounted = 0;
							ksms.UserdataMounted = 0;
						}
#endif // #ifdef KLOG_PATH_INTERNAL
						if(AllowCustomPath == 0)
						{
							ALOGE("Invalid path %s", argv[2]);
							AllowCustomFilename = 0;
						}
					}
//check customized filename
					if(AllowCustomFilename == 1)
					{
						for(i = 0; i < strlen(argv[2]); i++)
						{
							if(!strchr(KLOG_FILENAME_VALID_PATTERN, argv[2][i]))
							{
								ALOGE("Invalid filename %s", argv[2]);
								AllowCustomPath = 0;
								AllowCustomFilename = 0;
								break;
							}
						}
					}
					if(AllowCustomFilename == 1)
					{
						strncpy(fname, argv[2], sizeof(fname) - 1);
						if(fname[strlen(fname) - 1] == '/')
						{
							ALOGE("Invalid filename %s", argv[2]);
							AllowCustomPath = 0;
							AllowCustomFilename = 0;
						}
					}
//set customized filename
					if(AllowCustomFilename == 1)
					{
						if(strrchr(fname, '/'))
						{
							strncpy(fname, strrchr(fname, '/') + 1, 255);
						}
					}
//make sure the directory has been created
					if(AllowCustomPath == 1)
					{
						strncpy(pname, argv[2], sizeof(pname) - 1);
						pname[strrchr(pname, '/') - pname] = '\0';
						snprintf(cmdline, sizeof(cmdline), "mkdir -p %s", pname);
						system(cmdline);
					}
				}

//set default filename
				if(strlen(fname) == 0)
				{
					snprintf(fname, sizeof(fname), "%s%s%s", KLOG_FILENAME, timestamp, KLOG_FILENAME_EXT);
				}

//external storage
#ifdef KLOG_PATH_EXTERNAL
				if(ksms.ExternalMounted == 1)
				{
					ExternalSaved = 1;
					if(AllowCustomPath)
					{
						snprintf(cmdline, sizeof(cmdline), "%s/%s", pname, fname);
					}
					else
					{
						snprintf(cmdline, sizeof(cmdline), "/system/bin/mkdir -p %s", KLOG_PATH_EXTERNAL);
						system(cmdline);
						snprintf(cmdline, sizeof(cmdline), "%s/%s", KLOG_PATH_EXTERNAL, fname);
					}
					ret = dump_to_target(cmdline, KLOG_INDEX_INVALID);
					if(ret == KLOG_OK)
					{
						ALOGI("Write %s to external SD succeed", cmdline);
					}
					else
					{
						ALOGE("Write %s to external SD failed", cmdline);
						rc = KLOG_ERROR;
					}
				}
#endif // #ifdef KLOG_PATH_EXTERNAL

//internal storage
#ifdef KLOG_PATH_INTERNAL
				if(ksms.InternalMounted == 1)
				{
					InternalSaved = 1;
					if(AllowCustomPath)
					{
						snprintf(cmdline, sizeof(cmdline), "%s/%s", pname, fname);
					}
					else
					{
						snprintf(cmdline, sizeof(cmdline), "/system/bin/mkdir -p %s", KLOG_PATH_INTERNAL);
						system(cmdline);
						snprintf(cmdline, sizeof(cmdline), "%s/%s", KLOG_PATH_INTERNAL, fname);
					}
					ret = dump_to_target(cmdline, KLOG_INDEX_INVALID);
					if(ret == KLOG_OK)
					{
						ALOGI("Write %s to internal SD succeed", cmdline);
					}
					else
					{
						ALOGE("Write %s to internal SD failed", cmdline);
						rc = KLOG_ERROR;
					}
				}
#endif // #ifdef KLOG_PATH_INTERNAL

//USERDATA
#ifdef KLOG_PATH_USERDATA
				if(InternalSaved == 0 && ksms.UserdataMounted == 1)
				{
//KLog count property for limiting KLog files which placed in USERDATA
					snprintf(cmdline, sizeof(cmdline), "/system/bin/mkdir -p %s", KLOG_PATH_USERDATA);
					system(cmdline);

					if(AllowCustomFilename)
					{
						snprintf(cmdline, sizeof(cmdline), "%s/%s", pname, fname);
					}
					else
					{
						property_get(KLOG_PROPERTY_NAME_MAX, klog_max, KLOG_PROPERTY_VALUE_MAX);
						if(atoi(klog_max) > KLOG_FILES_MAX)
						{
							snprintf(klog_max, sizeof(klog_max), "%d", KLOG_FILES_MAX);
							property_set(KLOG_PROPERTY_NAME_MAX, klog_max);
						}

						property_get(KLOG_PROPERTY_NAME_USERDATA_COUNT, klog_userdata_count, KLOG_PROPERTY_VALUE_USERDATA_COUNT);
						snprintf(klog_userdata_count, sizeof(klog_userdata_count), "%d", (atoi(klog_userdata_count) + 1) % atoi(klog_max));
						property_set(KLOG_PROPERTY_NAME_USERDATA_COUNT, klog_userdata_count);

						snprintf(cmdline, sizeof(cmdline), "/system/bin/rm %s/%s%s-*", KLOG_PATH_USERDATA, KLOG_FILENAME, klog_userdata_count);
						system(cmdline);
						ALOGI("Remove old KLog files %s from USERDATA", cmdline);

						snprintf(cmdline, sizeof(cmdline), "%s/%s%s-%s%s", KLOG_PATH_USERDATA, KLOG_FILENAME, klog_userdata_count, timestamp, KLOG_FILENAME_EXT);
					}
					ret = dump_to_target(cmdline, KLOG_INDEX_INVALID);
					if(ret == KLOG_OK)
					{
						ALOGI("Write %s to USERDATA succeed", cmdline);
					}
					else
					{
						ALOGE("Write %s to USERDATA failed", cmdline);
						rc = KLOG_ERROR;
					}
				}
#endif // #ifdef KLOG_PATH_USERDATA
			}
			else if(strcmp(argv[1], KLOG_COMMAND_CLEAN_KLOG_FILES) == 0)
			{
#ifdef KLOG_PATH_USERDATA
				snprintf(cmdline, sizeof(cmdline), "/system/bin/rm -r %s/*", KLOG_PATH_USERDATA);
				system(cmdline);
				ALOGI("Clean all KLog files from USERDATA");
#endif // #ifdef KLOG_PATH_USERDATA
#ifdef KLOG_PATH_INTERNAL
				snprintf(cmdline, sizeof(cmdline), "/system/bin/rm -r %s/*", KLOG_PATH_INTERNAL);
				system(cmdline);
				ALOGI("Clean all KLog files from internal SD");
#endif // #ifdef KLOG_PATH_INTERNAL
#ifdef KLOG_PATH_EXTERNAL
				snprintf(cmdline, sizeof(cmdline), "/system/bin/rm -r %s/*", KLOG_PATH_EXTERNAL);
				system(cmdline);
				ALOGI("Clean all KLog files from external SD");
#endif // #ifdef KLOG_PATH_EXTERNAL

				rc = KLOG_OK;
			}

#ifdef CCI_KLOG_ALLOW_FORCE_PANIC
			else if(strcmp(argv[1], KLOG_COMMAND_TRIGGER_KERNEL_PANIC) == 0)
			{
				ALOGI("klogcat triggers kernel panic, called by %s(pid:%d)", caller, klogcat_ppid);
				return klogcat_panic();
			}

			else if(strcmp(argv[1], KLOG_COMMAND_TRIGGER_KERNEL_CRASH) == 0)
			{
				ALOGI("klogcat triggers system crash, called by %s(pid:%d)", caller, klogcat_ppid);
				return klogcat_crash();
			}

			else if(strcmp(argv[1], KLOG_COMMAND_SET_SUSPEND_PANIC) == 0)
			{
				if(argv[2] && strcmp(argv[2], "1") == 0)
				{
					rc = 1;
				}
				else
				{
					rc = 0;
				}
				ALOGI("klogcat set suspend triggering kernel panic, enable=%d, called by %s(pid:%d)", rc, caller, klogcat_ppid);
				rc = klogcat_panic_when_suspend(rc);
			}

			else if(strcmp(argv[1], KLOG_COMMAND_SET_POWEROFF_PANIC) == 0)
			{
				if(argv[2] && strcmp(argv[2], "1") == 0)
				{
					rc = 1;
				}
				else
				{
					rc = 0;
				}
				ALOGI("klogcat set power off triggering kernel panic, enable=%d, called by %s(pid:%d)", rc, caller, klogcat_ppid);
				rc = klogcat_panic_when_power_off(rc);
			}

			else if(strcmp(argv[1], KLOG_COMMAND_SET_MAGIC) == 0)
			{
				if(argv[2] && strlen(argv[2]) == KLOG_MAGIC_LENGTH)
				{
					ALOGI("klogcat set magic to %s, called by %s(pid:%d)", argv[2], caller, klogcat_ppid);
					rc = klogcat_set_magic(argv[2]);
				}
				else
				{
					ALOGE("Invalid klog magic");
				}
			}

			else if(strcmp(argv[1], KLOG_COMMAND_TRIGGER_USER_NULLPOINTER) == 0)
			{
				ALOGI("klogcat simulates userspace null pointer exception, called by %s(pid:%d)", caller, klogcat_ppid);
				klogcat_ppid = fork();
				if(klogcat_ppid == 0)
				{
					int *p = 0;
					ALOGI("%d:child suicide by null pointer exception", getpid());
					*p = 0;
					ALOGI("%d:child suicide failed, exit", getpid());
					exit(1);
				}
				else
				{
					ALOGI("%d:waiting for child %d", getpid(), klogcat_ppid);
					klogcat_ppid = wait(&rc);
				}
				if(klogcat_ppid)
				{
					ALOGI("%d:wait child %d finished", getpid(), klogcat_ppid);
				}
			}

			else if(strcmp(argv[1], KLOG_COMMAND_TRIGGER_USER_DIV0) == 0)
			{
				ALOGI("klogcat simulates userspace div0 exception, called by %s(pid:%d)", caller, klogcat_ppid);
				klogcat_ppid = fork();
				if(klogcat_ppid == 0)
				{
					ALOGI("%d:child suicide by div0", getpid());
					rc = 1 / 0;
					ALOGI("%d:child suicide failed, exit", getpid());
					exit(1);
				}
				else
				{
					ALOGI("%d:waiting for child %d", getpid(), klogcat_ppid);
					klogcat_ppid = wait(&rc);
				}
				if(klogcat_ppid)
				{
					ALOGI("%d:wait child %d finished", getpid(), klogcat_ppid);
				}
			}
#endif // #ifdef CCI_KLOG_ALLOW_FORCE_PANIC
		}
	}
	else if(KlogDumpMode)
	{
		rc = do_klogcat(1, KLOG_INDEX_INVALID);
	}

	return rc;
}

