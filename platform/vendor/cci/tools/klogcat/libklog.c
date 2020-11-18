#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/properties.h>

#define LOG_NIDEBUG 0
#define LOG_TAG "libklog"
#include <utils/Log.h>
#include <sys/ptrace.h>
#include <log/log.h>

#include "libklog.h"
#include "cciklog.h"

unsigned int log_type[] =
{
#if CCI_KLOG_CRASH_SIZE
	KLOG_IOCTL_GET_CRASH,
#endif // #if CCI_KLOG_CRASH_SIZE
#if CCI_KLOG_APPSBL_SIZE
	KLOG_IOCTL_GET_APPSBL,
#endif // #if CCI_KLOG_APPSBL_SIZE
#if CCI_KLOG_KERNEL_SIZE
	KLOG_IOCTL_GET_KERNEL,
#endif // #if CCI_KLOG_KERNEL_SIZE
#if CCI_KLOG_ANDROID_MAIN_SIZE
	KLOG_IOCTL_GET_ANDROID_MAIN,
#endif // #if CCI_KLOG_ANDROID_MAIN_SIZE
#if CCI_KLOG_ANDROID_SYSTEM_SIZE
	KLOG_IOCTL_GET_ANDROID_SYSTEM,
#endif // #if CCI_KLOG_ANDROID_SYSTEM_SIZE
#if CCI_KLOG_ANDROID_RADIO_SIZE
	KLOG_IOCTL_GET_ANDROID_RADIO,
#endif // #if CCI_KLOG_ANDROID_RADIO_SIZE
#if CCI_KLOG_ANDROID_EVENTS_SIZE
	KLOG_IOCTL_GET_ANDROID_EVENTS,
#endif // #if CCI_KLOG_ANDROID_EVENTS_SIZE
};

int perform_klog_ioctl(unsigned int ioctl_code, void* arg)
{
	int dev_fd;

	dev_fd = open(KLOG_DEV_PATH, O_RDONLY);
	if(dev_fd == -1)
	{
		ALOGE("Can't open %s", KLOG_DEV_PATH);
		printf("Can't open %s\n", KLOG_DEV_PATH);

		return KLOG_ERROR;
	}

	ioctl(dev_fd, ioctl_code, arg);
	close(dev_fd);

	return KLOG_OK;
}

int prepare_sysinfo(struct system_information *sysinfo)
{
	char klog_userdata_count[PROPERTY_VALUE_MAX] = {0};
	char klog_internal_count[PROPERTY_VALUE_MAX] = {0};
	char klog_external_count[PROPERTY_VALUE_MAX] = {0};
	char normal_boot[PROPERTY_VALUE_MAX] = {0};
	char android_version[PROPERTY_VALUE_MAX] = {0};
	char modem_version[PROPERTY_VALUE_MAX] = {0};
	char flex_version[PROPERTY_VALUE_MAX] = {0};
//	char linux_version[PROPERTY_VALUE_MAX] = {0};
	char version_id[PROPERTY_VALUE_MAX] = {0};
	char build_version[PROPERTY_VALUE_MAX] = {0};
	char build_date[PROPERTY_VALUE_MAX] = {0};
	char build_type[PROPERTY_VALUE_MAX] = {0};
	char build_user[PROPERTY_VALUE_MAX] = {0};
	char build_host[PROPERTY_VALUE_MAX] = {0};
	char build_key[PROPERTY_VALUE_MAX] = {0};
	char secure_mode[PROPERTY_VALUE_MAX] = {0};
	char debug_mode[PROPERTY_VALUE_MAX] = {0};
	char product_model[PROPERTY_VALUE_MAX] = {0};
	char cpuinfo_max_freq[KLOG_CPUINFO_MAX_FREQ_LENGTH] = {0};
	char scaling_max_freq[KLOG_SCALING_MAX_FREQ_LENGTH] = {0};
//	char memory_info[30] = {0};
//	char commandline_info[100] = {0};
	char sim_state[PROPERTY_VALUE_MAX] = {0};
	FILE *fp = NULL;
	char line[1024] = {0};
	char *temppos = NULL;
	int ret = KLOG_OK;

//static info

//[persist.klog.userdata.count]: [2]
	property_get(KLOG_PROPERTY_NAME_USERDATA_COUNT, klog_userdata_count, KLOG_PROPERTY_VALUE_USERDATA_COUNT);
	strncpy(sysinfo->klog_userdata_count, klog_userdata_count, KLOG_USERDATA_COUNT_LENGTH - 1);
//	ALOGI("klog_userdata_count=%s", sysinfo->klog_userdata_count);

//[persist.klog.internal.count]: [2]
	property_get(KLOG_PROPERTY_NAME_INTERNAL_COUNT, klog_internal_count, KLOG_PROPERTY_VALUE_INTERNAL_COUNT);
	strncpy(sysinfo->klog_internal_count, klog_internal_count, KLOG_INTERNAL_COUNT_LENGTH - 1);
//	ALOGI("klog_internal_count=%s", sysinfo->klog_internal_count);

//[persist.klog.external.count]: [2]
	property_get(KLOG_PROPERTY_NAME_EXTERNAL_COUNT, klog_external_count, KLOG_PROPERTY_VALUE_EXTERNAL_COUNT);
	strncpy(sysinfo->klog_external_count, klog_external_count, KLOG_EXTERNAL_COUNT_LENGTH - 1);
//	ALOGI("klog_external_count=%s", sysinfo->klog_external_count);

//commandline info
//[ro.normalboot]: [1]
	property_get("ro.normalboot", normal_boot, "-");
	strncpy(sysinfo->normal_boot, normal_boot, KLOG_NORMAL_BOOT_LENGTH - 1);
//	ALOGI("Normal boot=%s", sysinfo->normal_boot);

//modem version
//[gsm.version.baseband]: [DA80_0.01-006_ICS]
//DA80.038d
//8930B-BAAAANAZQ-12321-05-2014
	property_get("gsm.version.baseband", modem_version, "-");
	temppos = strchr(modem_version, ',');
	if(temppos)
	{
		temppos[0] = 0;
		temppos = NULL;
	}
	strncpy(sysinfo->modem_version, modem_version, KLOG_MODEM_VERSION_LENGTH - 1);
//	ALOGI("modem version=%s", sysinfo->modem_version);

//Linux version
//cat /proc/version
//Linux version 2.6.35.11-perf+ (cme@androidbs-desktop) (gcc version 4.4.3 (GCC) ) #1 SMP PREEMPT Fri Aug 26 15:51:05 CST 2011
//Linux version 3.4.0 (jimmy@Jimmy-HP-Compaq-6200-Pro-MT-PC) (gcc version 4.7 (GCC) ) #1 SMP PREEMPT Thu Dec 26 21:05:03 CST 2013
	fp = fopen("/proc/version", "r");
	if(fp)
	{
		fgets(line, sizeof(line), fp);
		line[strlen(line)] = 0;
//		strncpy(sysinfo->linux_version, &line[14], strchr(line, '(') - line - 14);//after "Linux version " and before "("
		strncpy(sysinfo->linux_version, &line[14], KLOG_LINUX_VERSION_LENGTH - 1);//after "Linux version "
		fclose(fp);
		fp = NULL;
	}

//debug mode
//[ro.enter_dl_mode]: [1]
	property_get("ro.enter_dl_mode", debug_mode, "-");
	strncpy(sysinfo->debug_mode, debug_mode, KLOG_DEBUG_MODE_LENGTH - 1);
//	ALOGI("debug mode=%s", sysinfo->debug_mode);

//android version
//[ro.build.version.release]: [4.0.3]
	property_get("ro.build.version.release", android_version, "-");
	strncpy(sysinfo->android_version, android_version, KLOG_ANDROID_VERSION_LENGTH - 1);
//	ALOGI("android version=%s", sysinfo->android_version);

//build version
//[ro.version]: [DA80.023]
//[ro.build.version.incremental]: [Android.SA77.0.1.1014]
//[ro.build.version.incremental]: [eng.jimmy.20131226.202750]
	property_get("ro.build.version.incremental", build_version, "-");
	strncpy(sysinfo->build_version, build_version, KLOG_BUILD_VERSION_LENGTH - 1);
//	ALOGI("Build version=%s", sysinfo->build_version);

//Flex version
//[ro.cci.flex_version]: [FF_DA80_Factory_023]
	property_get("ro.cci.flex_version", flex_version, "-");
	strncpy(sysinfo->flex_version, flex_version, KLOG_FLEX_VERSION_LENGTH - 1);
//	ALOGI("Flex version=%s", sysinfo->flex_version);

//Version ID
//[ro.build.display.id]: [GRJ90]
//[ro.build.display.id]: [046-World Wide-Dell]
//[ro.build.display.id]: [eagle_ds-userdebug 4.3 JSS15Q eng.jimmy.20131226.202750 test-keys]
	property_get("ro.build.display.id", version_id, "-");
	strncpy(sysinfo->version_id, version_id, KLOG_VERSION_ID_LENGTH - 1);
//	ALOGI("Version ID=%s", sysinfo->version_id);

//[ro.build.date]: [Thu Nov  3 21:25:19 CST 2011] [一 11月  7 15:13:14 CST 2011]
//[ro.build.date.utc]: [1320649994]
	property_get("ro.build.date", build_date, "-");
	strncpy(sysinfo->build_date, build_date, KLOG_BUILD_DATE_LENGTH - 1);
//	ALOGI("Build date=%s, length=%d", sysinfo->build_date, strlen(sysinfo->build_date));

//[ro.build.type]: [eng] [user] [userdebug]
	property_get("ro.build.type", build_type, "-");
	strncpy(sysinfo->build_type, build_type, KLOG_BUILD_TYPE_LENGTH - 1);
//	ALOGI("Build type=%s", sysinfo->build_type);

//[ro.build.user]: [jimmy]
	property_get("ro.build.user", build_user, "-");
	strncpy(sysinfo->build_user, build_user, KLOG_BUILD_USER_LENGTH - 1);
//	ALOGI("Build user=%s", sysinfo->build_user);

//[ro.build.host]: [androidbs-desktop]
	property_get("ro.build.host", build_host, "-");
	strncpy(sysinfo->build_host, build_host, KLOG_BUILD_HOST_LENGTH - 1);
//	ALOGI("Build host=%s", sysinfo->build_host);

//[ro.build.tags]: [test-keys] [release-keys]
	property_get("ro.build.tags", build_key, "-");
	strncpy(sysinfo->build_key, build_key, KLOG_BUILD_KEY_LENGTH - 1);
//	ALOGI("Build key=%s", sysinfo->build_key);

//[ro.product.device]: [streakpro]
//[ro.product.device]: [sa77]
//[ro.product.device]: [D2303]
	property_get("ro.product.device", product_model, "-");
	strncpy(sysinfo->product_model, product_model, KLOG_PRODUCT_MODEL_LENGTH - 1);
//	ALOGI("product model=%s", sysinfo->product_model);

//[ro.secure]: [0]
	property_get("ro.secure", secure_mode, "-");
	strncpy(sysinfo->secure_mode, secure_mode, KLOG_SECURE_MODE_LENGTH - 1);
//	ALOGI("Secure mode=%s", sysinfo->secure_mode);

//CPU info
//cat /proc/cpuinfo
//cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq
	if(!(fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r")))
	{
		ALOGE("Error opening cpuinfo_max_freq");
		printf("Error opening cpuinfo_max_freq\n");
	}
	else
	{
		while(fgets(line, sizeof(line), fp))
		{
			line[strlen(line)] = '\0';
			sscanf(line, "%7s\n", cpuinfo_max_freq);
			strncpy(sysinfo->cpuinfo_max_freq, cpuinfo_max_freq, KLOG_CPUINFO_MAX_FREQ_LENGTH - 1);
		}
		fclose(fp);
		fp = NULL;
	}

//cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
	if(!(fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r")))
	{
		ALOGE("Error opening scaling_max_freq");
		printf("Error opening scaling_max_freq\n");
	}
	else
	{
		while(fgets(line, sizeof(line), fp))
		{
			line[strlen(line)] = '\0';
			sscanf(line, "%7s\n", scaling_max_freq);
			strncpy(sysinfo->scaling_max_freq, scaling_max_freq, KLOG_SCALING_MAX_FREQ_LENGTH - 1);
		}
		fclose(fp);
		fp = NULL;
	}

//memory info
//cat /proc/meminfo

//dynamic info

//[gsm.sim.state]: [ABSENT] [PIN_REQUIRED] [READY] [NOT_READY]
	property_get("gsm.sim.state", sim_state, "-");
	strncpy(sysinfo->sim_state, sim_state, KLOG_SIM_STATE_LENGTH - 1);
//	ALOGI("SIM state=%s", sysinfo->sim_state);

	return ret;
}

int record_sysinfo(int system_on, int system_state)
{
	struct system_information sysinfo;

	memset(&sysinfo, 0, sizeof(struct system_information));

	if(system_on > 0)
	{
		prepare_sysinfo(&sysinfo);
	}

//system on-off state
	sysinfo.system_on_off_state = system_on * 0x10 + system_state;

	return perform_klog_ioctl(KLOG_IOCTL_RECORD_SYSINFO, &sysinfo);
}

int get_sysinfo(struct system_information* sysinfo)
{
	char normal_boot[PROPERTY_VALUE_MAX] = {0};
	char sysinfo_inited[PROPERTY_VALUE_MAX] = {0};
	char klog_userdata_count[PROPERTY_VALUE_MAX] = {0};
	char klog_internal_count[PROPERTY_VALUE_MAX] = {0};
	char klog_external_count[PROPERTY_VALUE_MAX] = {0};

	int rc = KLOG_OK;

	property_get("ro.normalboot", normal_boot, "-");
	property_get("sys.klog.sysinfo.inited", sysinfo_inited, "-");
	if(strcmp(normal_boot, "3") == 0 && strcmp(sysinfo_inited, "-") == 0)//charge-only mode and sysinfo not inited yet, simulate klogcat init
	{
		record_sysinfo(1, 0);
		property_set("sys.klog.sysinfo.inited", "1");
	}

	rc = perform_klog_ioctl(KLOG_IOCTL_GET_HEADER, sysinfo);

	if(rc == KLOG_OK)
	{
//check klog signature
		if(strcmp(sysinfo->klog_signature, KLOG_SIGNATURE) != 0)
		{
			ALOGE("Invalid klog buffer");
			printf("Invalid klog buffer\n");
			rc = KLOG_ERROR;
		}
		else
		{
//overwrite klog count with properties
//[persist.klog.userdata.count]: [2]
			property_get(KLOG_PROPERTY_NAME_USERDATA_COUNT, klog_userdata_count, KLOG_PROPERTY_VALUE_USERDATA_COUNT);
			strncpy(sysinfo->klog_userdata_count, klog_userdata_count, KLOG_USERDATA_COUNT_LENGTH - 1);
//[persist.klog.internal.count]: [2]
			property_get(KLOG_PROPERTY_NAME_INTERNAL_COUNT, klog_internal_count, KLOG_PROPERTY_VALUE_INTERNAL_COUNT);
			strncpy(sysinfo->klog_internal_count, klog_internal_count, KLOG_INTERNAL_COUNT_LENGTH - 1);
//[persist.klog.external.count]: [2]
			property_get(KLOG_PROPERTY_NAME_EXTERNAL_COUNT, klog_external_count, KLOG_PROPERTY_VALUE_EXTERNAL_COUNT);
			strncpy(sysinfo->klog_external_count, klog_external_count, KLOG_EXTERNAL_COUNT_LENGTH - 1);
		}
	}

	return rc;
}

void dump_sysinfo(struct system_information* sysinfo, int fd)
{
	FILE *fp;
	char line[1024] = {0};
	char prev_reboot_reason[KLOG_MAGIC_LENGTH + 1] = {0};
	char prev_normal_boot[KLOG_NORMAL_BOOT_LENGTH + 1] = {0};
	char klog_max[PROPERTY_VALUE_MAX] = {0};
	char strbuf[1024] = {0};
	unsigned int len = 0;

//increase klog count if crashed
	if(!strncmp(sysinfo->magic, KLOG_MAGIC_MARM_FATAL, strlen(KLOG_MAGIC_MARM_FATAL)) || !strncmp(sysinfo->magic, KLOG_MAGIC_AARM_PANIC, strlen(KLOG_MAGIC_AARM_PANIC)) || !strncmp(sysinfo->magic, KLOG_MAGIC_RPM_CRASH, strlen(KLOG_MAGIC_RPM_CRASH)) || !strncmp(sysinfo->magic, KLOG_MAGIC_SUBSYS_CRASH, strlen(KLOG_MAGIC_SUBSYS_CRASH)) || !strncmp(sysinfo->magic, KLOG_MAGIC_FIQ_HANG, strlen(KLOG_MAGIC_FIQ_HANG)) || !strncmp(sysinfo->magic, KLOG_MAGIC_UNKNOWN_CRASH, strlen(KLOG_MAGIC_UNKNOWN_CRASH)))
	{
		property_get(KLOG_PROPERTY_NAME_MAX, klog_max, KLOG_PROPERTY_VALUE_MAX);
		if(atoi(klog_max) > KLOG_FILES_MAX)
		{
			snprintf(klog_max, sizeof(klog_max), "%d", KLOG_FILES_MAX);
			property_set(KLOG_PROPERTY_NAME_MAX, klog_max);
		}
		snprintf(sysinfo->klog_userdata_count, sizeof(sysinfo->klog_userdata_count), "%d", (atoi(sysinfo->klog_userdata_count) + 1) % atoi(klog_max));
		snprintf(sysinfo->klog_internal_count, sizeof(sysinfo->klog_internal_count), "%d", (atoi(sysinfo->klog_internal_count) + 1) % atoi(klog_max));
		snprintf(sysinfo->klog_external_count, sizeof(sysinfo->klog_external_count), "%d", (atoi(sysinfo->klog_external_count) + 1) % atoi(klog_max));
	}

//cat /sys/class/misc/cciklog/prev_reboot_reason
	if(!(fp = fopen("/sys/class/misc/cciklog/prev_reboot_reason", "r")))
	{
		ALOGE("Error opening prev_reboot_reason");
		printf("Error opening prev_reboot_reason\n");
	}
	while(fgets(line, sizeof(line), fp))
	{
		line[strlen(line)] = '\0';
		sscanf(line, "%6s\n", prev_reboot_reason);
	}
	fclose(fp);
//cat /sys/class/misc/cciklog/prev_normal_boot
	if(!(fp = fopen("/sys/class/misc/cciklog/prev_normal_boot", "r")))
	{
		ALOGE("Error opening prev_normal_boot");
		printf("Error opening prev_normal_boot\n");
	}
	while(fgets(line, sizeof(line), fp))
	{
		line[strlen(line)] = '\0';
		sscanf(line, "%6s\n", prev_normal_boot);
	}
	fclose(fp);

	len = snprintf(strbuf, sizeof(strbuf), "============================= KLog Information =============================\n");
	write(fd, strbuf, len);
	len = snprintf(strbuf, sizeof(struct system_information), "header version:%08X\nversion:%08X\n", sysinfo->klog_header_version, sysinfo->klog_version);
	write(fd, strbuf, len);
	len = snprintf(strbuf, sizeof(struct system_information), "magic:%s\nprev_reboot_reason:%s\nkernel_time:%s\nfirst_rtc:%s\nlast_rtc:%s\nnormal_boot:%s\nprev_normal_boot:%s\nklog_userdata_count:%s\nklog_internal_count:%s\nklog_external_count:%s\n", sysinfo->magic, prev_reboot_reason, sysinfo->kernel_time, sysinfo->first_rtc, sysinfo->last_rtc, sysinfo->normal_boot, prev_normal_boot, sysinfo->klog_userdata_count, sysinfo->klog_internal_count, sysinfo->klog_external_count);
	write(fd, strbuf, len);
	len = snprintf(strbuf, sizeof(struct system_information), "system_on_off_state:0x%X\nsbl_bootup_time:%u\naboot_bootup_time:%u\nandroid_bootup_time:%u\nandroid_shutdown_time:%u\nsysfs_sync_time:%u\nkernel_power_off_time:%u\nsuspend_time:%u\nresume_time:%u\n", sysinfo->system_on_off_state, sysinfo->sbl_bootup_time, sysinfo->aboot_bootup_time, sysinfo->android_bootup_time, sysinfo->android_shutdown_time, sysinfo->sysfs_sync_time, sysinfo->kernel_power_off_time, sysinfo->suspend_time, sysinfo->resume_time);
	write(fd, strbuf, len);
	len = snprintf(strbuf, sizeof(struct system_information), "android_version:%s\nmodem_version:%s\nflex_version:%s\nrpm_version:%s\nbootloader_version:%s\nlinux_version:%s\nversion_id:%s\nbuild_version:%s\n", sysinfo->android_version, sysinfo->modem_version, sysinfo->flex_version, sysinfo->rpm_version, sysinfo->bootloader_version, sysinfo->linux_version, sysinfo->version_id, sysinfo->build_version);
	write(fd, strbuf, len);
	len = snprintf(strbuf, sizeof(struct system_information), "build_date:%s\nbuild_type:%s\nbuild_user:%s\nbuild_host:%s\nbuild_key:%s\nsecure_mode:%s\ndebug_mode:%s\n", sysinfo->build_date, sysinfo->build_type, sysinfo->build_user, sysinfo->build_host, sysinfo->build_key, sysinfo->secure_mode, sysinfo->debug_mode);
	write(fd, strbuf, len);
	len = snprintf(strbuf, sizeof(struct system_information), "product_model:%s\nhw_id:%s\ncpuinfo_max_freq:%s\nscaling_max_freq:%s\nsim_state:%s\n", sysinfo->product_model, sysinfo->hw_id, sysinfo->cpuinfo_max_freq, sysinfo->scaling_max_freq, sysinfo->sim_state);
	write(fd, strbuf, len);

	return;
}

void dump_log(struct klog_category* pklog_category, int category, int fd)
{
	unsigned int len = 0;
	char strbuf[80] = {0};

	len = snprintf(strbuf, sizeof(strbuf), "\n============================= KLog Start =============================\n");
	write(fd, strbuf, len);
	len = snprintf(strbuf, sizeof(strbuf), "KLog Category #%u:\nname:%s\nsize:%u\nindex:%u\noverload:%u\n", category, pklog_category->name, pklog_category->size, pklog_category->index, pklog_category->overload);
	write(fd, strbuf, len);

	if(pklog_category->overload == 0 && pklog_category->index == 0)
	{
		len = snprintf(strbuf, sizeof(strbuf), "<Empty>\n");
		write(fd, strbuf, len);
	}
	else
	{
		if(pklog_category->overload == 0)
		{
			write(fd, &pklog_category->buffer[0], pklog_category->index);
			len = pklog_category->index - 1;
		}
		else
		{
			write(fd, &pklog_category->buffer[pklog_category->index], (pklog_category->size - KLOG_CATEGORY_HEADER_SIZE) - pklog_category->index);
			write(fd, &pklog_category->buffer[0], pklog_category->index);
			len = (pklog_category->index != 0) ? (pklog_category->index - 1) : (pklog_category->size - KLOG_CATEGORY_HEADER_SIZE - 1);
		}
		if(pklog_category->buffer[len] != '\n' && pklog_category->buffer[len] != '\r')
		{
			write(fd, "\n", 1);
		}
	}

	len = snprintf(strbuf, sizeof(strbuf), "============================== KLog End ==============================\n");
	write(fd, strbuf, len);

	return;
}

int get_log(struct klog_category* pklog_category, int category)
{
	return perform_klog_ioctl(log_type[category], pklog_category);
}

int do_klogcat(int fd, int dump_crash)
{
	struct system_information sysinfo;
	struct klog_category* pklog_category;
	unsigned int len = 0;
	char strbuf[80] = {0};
	int ret = KLOG_OK;

	int i = 0;

//sysinfo
	ret = get_sysinfo(&sysinfo);
	if(ret != KLOG_ERROR)
	{
		dump_sysinfo(&sysinfo, fd);

//dump all categories
		for(i = 0; i < KLOG_IGNORE; i++)
		{
			pklog_category = malloc(sizeof(struct klog_category) + get_max_category_size() - 1);
			if((ret = get_log(pklog_category, i)) != KLOG_OK)
			{
				free(pklog_category);
				return ret;
			}
			dump_log(pklog_category, i, fd);
			free(pklog_category);
		}

//dump process snapshot
		if(dump_crash != KLOG_INDEX_AARM_PANIC && dump_crash != KLOG_INDEX_RPM_CRASH && dump_crash != KLOG_INDEX_FIQ_HANG && dump_crash != KLOG_INDEX_UNKNOWN_CRASH)
		{
			pklog_category = malloc(sizeof(struct klog_category) + get_max_category_size() - 1);
			get_process_info(pklog_category);
			dump_log(pklog_category, KLOG_IGNORE, fd);//Be aware! PS is not in cklc_category.
			free(pklog_category);
		}
	}

	return ret;
}

int dump_to_target(const char* target_path, int dump_crash)
{
	char cmdline[256] = {0};
	int ret = KLOG_OK;
	int fd = 0;
#ifdef CCI_KLOG_ALLOW_FORCE_PANIC
#if CCI_KLOG_CRASH_SIZE
#ifdef CCI_KLOG_SUPPORT_MTBF
#ifdef KLOG_PATH_USERDATA_PARTITION
	struct klog_category* pklog_category;
	char crashlog[256] = {0};
	char MTBF_state[PROPERTY_VALUE_MAX] = {0};
	char version_id[PROPERTY_VALUE_MAX] = {0};
	unsigned int len = 0;
	int i = 0;
#endif // #ifdef KLOG_PATH_USERDATA_PARTITION
#endif // #ifdef CCI_KLOG_SUPPORT_MTBF
#endif // #if CCI_KLOG_CRASH_SIZE
#endif // #ifdef CCI_KLOG_ALLOW_FORCE_PANIC

	fd = open(target_path, O_RDWR | O_CREAT, 0666);
	if(fd >= 0)
	{
		ret = do_klogcat(fd, dump_crash);
		if(ret == KLOG_OK)
		{
//sync
			while(fsync(fd) < 0)
			{
				ALOGE("fsync failed");
				printf("fsync failed\n");
				sleep(1);
			}
		}
		close(fd);
//pack
		snprintf(cmdline, sizeof(cmdline), "/system/bin/gzip %s", target_path);
		system(cmdline);
		snprintf(cmdline, sizeof(cmdline), "chmod 0666 %s%s", target_path, KLOG_FILENAME_EXT_GZIP);
		system(cmdline);
#ifdef CCI_KLOG_ALLOW_FORCE_PANIC
#if CCI_KLOG_CRASH_SIZE
#ifdef CCI_KLOG_SUPPORT_MTBF
#ifdef KLOG_PATH_USERDATA_PARTITION
		if(dump_crash == KLOG_INDEX_MARM_FATAL || dump_crash == KLOG_INDEX_AARM_PANIC || dump_crash == KLOG_INDEX_RPM_CRASH || dump_crash == KLOG_INDEX_SUBSYS_CRASH || dump_crash == KLOG_INDEX_FIQ_HANG || dump_crash == KLOG_INDEX_UNKNOWN_CRASH)
		{
//check MTBF state
			property_get("persist.sys.mtbf.enable", MTBF_state, "0");
			if(strcmp(MTBF_state, "1") == 0)
			{
//check USERDATA dumped
				strncpy(cmdline, KLOG_PATH_USERDATA, strlen(KLOG_PATH_USERDATA) + 1);
				strncat(cmdline, "/", strlen("/"));
				if(strncmp(target_path, cmdline, strlen(cmdline)) == 0)
				{
//create directory
					strncpy(crashlog, KLOG_PATH_USERDATA_PARTITION, strlen(KLOG_PATH_USERDATA_PARTITION) + 1);
					strncat(crashlog, "/crash", strlen("/crash"));
					snprintf(cmdline, sizeof(cmdline), "/system/bin/mkdir -p %s", crashlog);
					system(cmdline);
//get SW version
					property_get("ro.build.display.id", version_id, "-");
//dump file
					strncpy(crashlog, KLOG_PATH_USERDATA_PARTITION, strlen(KLOG_PATH_USERDATA_PARTITION) + 1);
					strncat(crashlog, "/crash", strlen("/crash"));
					strncat(crashlog, &target_path[strlen(KLOG_PATH_USERDATA)], 256 - strlen(KLOG_PATH_USERDATA) - strlen("/crash") - 1);
					fd = open(crashlog, O_RDWR | O_CREAT, 0666);
					if(fd >= 0)
					{
						pklog_category = malloc(sizeof(struct klog_category) + get_max_category_size() - 1);
						if((ret = get_log(pklog_category, KLOG_CRASH)) == KLOG_OK)
						{
							len = snprintf(cmdline, sizeof(cmdline), "%s\n", version_id);
							write(fd, cmdline, len);
							if(pklog_category->overload == 0 && pklog_category->index == 0)
							{
								for(i = 0; i < sizeof(kml) / sizeof(struct klog_magic_list); i++)
								{
									if(dump_crash == kml[i].index)
									{
										dump_crash = i;
										break;
									}
								}
								len = snprintf(cmdline, sizeof(cmdline), "%s\n", kml[dump_crash].name);
								write(fd, cmdline, len);
							}
							else
							{
								if(pklog_category->overload == 0)
								{
									write(fd, &pklog_category->buffer[0], pklog_category->index);
								}
								else
								{
									write(fd, &pklog_category->buffer[pklog_category->index], (pklog_category->size - KLOG_CATEGORY_HEADER_SIZE) - pklog_category->index);
									write(fd, &pklog_category->buffer[0], pklog_category->index - 1);
								}
							}
						}
						free(pklog_category);
						close(fd);
						snprintf(cmdline, sizeof(cmdline), "chmod 0666 %s", crashlog);
						system(cmdline);
						ALOGI("Write %s to USERDATA succeed", cmdline);
					}
				}
			}
		}
#endif // #ifdef KLOG_PATH_USERDATA_PARTITION
#endif // #ifdef CCI_KLOG_SUPPORT_MTBF
#endif // #if CCI_KLOG_CRASH_SIZE
#endif // #ifdef CCI_KLOG_ALLOW_FORCE_PANIC
	}
	else
	{
		ret = KLOG_ERROR;
	}

	return ret;
}

int copy_file(const char* source, const char* target)
{
	FILE *fpr = NULL;
	FILE *fpw = NULL;
	char *buffer = NULL;
	long length = 0;
	int ret = KLOG_OK;

	fpr = fopen(source, "rb");
	if(fpr)
	{
		fseek(fpr ,0, SEEK_END);
		length = ftell(fpr);
		if(length > 0)
		{
			fpw = fopen(target, "w+b");
			if(fpw)
			{
				buffer = (char *)malloc(length);
				if(buffer == NULL)
				{
					ALOGE("out of memory");
					printf("out of memory\n");
					ret = KLOG_ERROR;
					goto exit_copy_file;
				}
				fseek(fpr ,0, SEEK_SET);
				if(fread(buffer, length, 1, fpr) == 0)
				{
					ALOGE("read source file '%s' failed", source);
					printf("read source file '%s' failed\n", source);
					ret = KLOG_ERROR;
					goto exit_copy_file;
				}
				if(fwrite(buffer, length, 1, fpw) == 0)
				{
					ALOGE("write target file '%s' failed", target);
					printf("write target file '%s' failed\n", target);
					ret = KLOG_ERROR;
					goto exit_copy_file;
				}
			}
			else
			{
				ALOGE("open target file '%s' failed", target);
				printf("open target file '%s' failed\n", target);
				ret = KLOG_ERROR;
			}
		}
		else
		{
			ALOGE("source file '%s' length is zero", source);
			printf("source file '%s' length is zero\n", source);
			ret = KLOG_ERROR;
		}
	}
	else
	{
		ALOGE("open source file '%s' failed", source);
		printf("open source file '%s' failed\n", source);
		ret = KLOG_ERROR;
	}

exit_copy_file:
	if(fpw)
	{
		fclose(fpw);
		fpw = NULL;
	}
	if(fpr)
	{
		fclose(fpr);
		fpr = NULL;
	}
	if(buffer)
	{
		free(buffer);
		buffer = NULL;
	}
	return ret;
}

struct KSMS checkStorageMountStatus(void)
{
	struct KSMS ksms;
	char device[256] = {0};
	char mount_path[256] = {0};
	char rest[256] = {0};
	char line[1024] = {0};
	FILE *fp;

	ksms.UserdataMounted = 0;
	ksms.InternalMounted = 0;
	ksms.ExternalMounted = 0;

	if(!(fp = fopen("/proc/mounts", "r")))
	{
		ALOGE("Error opening /proc/mounts");
		printf("Error opening /proc/mounts\n");

		return ksms;
	}

	while(fgets(line, sizeof(line), fp))
	{
		line[strlen(line)] = '\0';
		sscanf(line, "%255s %255s %255s\n", device, mount_path, rest);
#ifdef KLOG_PATH_USERDATA_PARTITION
		if(!strcmp(mount_path, KLOG_PATH_USERDATA_PARTITION))
		{
			ksms.UserdataMounted = 1;
		}
#else // #ifdef KLOG_PATH_USERDATA_PARTITION
		ksms.UserdataMounted = -1;
#endif // #ifdef KLOG_PATH_USERDATA_PARTITION
#ifdef KLOG_PATH_INTERNAL_PARTITION
		if(!strcmp(mount_path, KLOG_PATH_INTERNAL_PARTITION))
		{
			ksms.InternalMounted = 1;
		}
#else // #ifdef KLOG_PATH_INTERNAL_PARTITION
		ksms.InternalMounted = -1;
#endif // #ifdef KLOG_PATH_INTERNAL_PARTITION
#ifdef KLOG_PATH_EXTERNAL_PARTITION
		if(!strcmp(mount_path, KLOG_PATH_EXTERNAL_PARTITION))
		{
			ksms.ExternalMounted = 1;
		}
#else // #ifdef KLOG_PATH_EXTERNAL_PARTITION
		ksms.ExternalMounted = -1;
#endif // #ifdef KLOG_PATH_EXTERNAL_PARTITION
	}

	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}

	return ksms;
}

int get_pid_name(pid_t pid, char* name)
{
	FILE *fp = NULL;
	char line[1024] = {0};
	char prev_reboot_reason[KLOG_MAGIC_LENGTH + 1] = {0};
	char prev_normal_boot[KLOG_NORMAL_BOOT_LENGTH + 1] = {0};
	char klog_max[PROPERTY_VALUE_MAX] = {0};
	unsigned int len = 0;
	int rc = KLOG_OK;

//cat /proc/klogcat_ppid/cmdline
	snprintf(line, sizeof(line), "/proc/%d/cmdline", pid);
	if(!(fp = fopen(line, "r")))
	{
		ALOGE("Error opening %s", line);
		printf("Error opening %s\n", line);
		rc = 1;
	}
	if(rc == 0)
	{
		while(fgets(line, sizeof(line), fp))
		{
			line[strlen(line)] = '\0';
			sscanf(line, "%s\n", name);
//			ALOGI("pid: %d, name: %s", pid, name);
		}
	}
	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}

	return rc;
}

