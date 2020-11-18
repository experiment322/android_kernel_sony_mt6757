#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <sys/time.h>
#include <linux/rtc.h>
#include <linux/android_alarm.h>
#include <malloc.h>

#define LOG_NIDEBUG 0
#define LOG_TAG "engmode"
#include <utils/Log.h>

#include "engmode.h"

void get_boot_information(int dev_fd)
{
	struct boot_information bootinfo;

#ifdef CCI_ENGMODE_WORKAROUND
	struct power_on_off_history pooh;
	struct power_on_off_counter pooc;
	FILE *fp = NULL;
	int current_index = 0;
#endif // #ifdef CCI_ENGMODE_WORKAROUND

	int i = 0;

	memset(&bootinfo, 0, sizeof(struct boot_information));
#ifdef CCI_ENGMODE_WORKAROUND
	memset(&pooh, 0, sizeof(struct power_on_off_history));
	memset(&pooc, 0, sizeof(struct power_on_off_counter));

	fp = fopen("/dev/block/mmcblk0p5", "rb");
	if(fp)
	{
//read power-on reason history
		fseek(fp, RSV_POWER_ON_OFF_REASON_ADDR + RSV_POWER_ON_REASON_RECENT_OFFSET, SEEK_SET);//0x12th-sector
		fread(&pooh, sizeof(struct power_on_off_history), 1, fp);
#ifdef CCI_ENGMODE_LOG
		ALOGI("power-on index=%u, reason=0x%X", pooh.index, pooh.reason[pooh.index]);
#endif // #ifdef CCI_ENGMODE_LOG
		for(i = 0; i < POWER_REASON_RECENT_LENGTH; i++)
		{
			current_index = (pooh.index - i) < 0 ? (pooh.index - i + POWER_REASON_RECENT_LENGTH) : (pooh.index - i);//ring buffer
#ifdef CCI_ENGMODE_LOG
			ALOGI("recent %d power-on reason:[%d]=0x%X", i, current_index, pooh.reason[current_index]);
#endif // #ifdef CCI_ENGMODE_LOG
			bootinfo.power_on_reason_recent[i] = pooh.reason[current_index];
		}
//read power-on reason counter
		fseek(fp, RSV_POWER_ON_OFF_REASON_ADDR + RSV_POWER_ON_REASON_COUNTER_OFFSET, SEEK_SET);//0x12th-sector and offset 0x50 bytes
		fread(&pooc, sizeof(struct power_on_off_counter), 1, fp);
		for(i = 0; i < POWER_REASON_COUNTER_LENGTH; i++)
		{
#ifdef CCI_ENGMODE_LOG
			ALOGI("power-on counter[%d]=%u", i, pooc.counter[i]);
#endif // #ifdef CCI_ENGMODE_LOG
			bootinfo.power_on_reason_counter[i] = pooc.counter[i];
		}
//read power-off reason history
		fseek(fp, RSV_POWER_ON_OFF_REASON_ADDR + RSV_POWER_OFF_REASON_RECENT_OFFSET, SEEK_SET);//0x12th-sector
		fread(&pooh, sizeof(struct power_on_off_history), 1, fp);
#ifdef CCI_ENGMODE_LOG
		ALOGI("power-off index=%u, reason=0x%X", pooh.index, pooh.reason[pooh.index]);
#endif // #ifdef CCI_ENGMODE_LOG
		for(i = 0; i < POWER_REASON_RECENT_LENGTH; i++)
		{
			current_index = (pooh.index - i) < 0 ? (pooh.index - i + POWER_REASON_RECENT_LENGTH) : (pooh.index - i);//ring buffer
#ifdef CCI_ENGMODE_LOG
			ALOGI("recent %d power-off reason:[%d]=0x%X", i, current_index, pooh.reason[current_index]);
#endif // #ifdef CCI_ENGMODE_LOG
			bootinfo.power_off_reason_recent[i] = pooh.reason[current_index];
		}
//read power-off reason counter
		fseek(fp, RSV_POWER_ON_OFF_REASON_ADDR + RSV_POWER_OFF_REASON_COUNTER_OFFSET, SEEK_SET);//0x12th-sector and offset 0x50 bytes
		fread(&pooc, sizeof(struct power_on_off_counter), 1, fp);
		for(i = 0; i < POWER_REASON_COUNTER_LENGTH; i++)
		{
#ifdef CCI_ENGMODE_LOG
			ALOGI("power-off counter[%d]=%u", i, pooc.counter[i]);
#endif // #ifdef CCI_ENGMODE_LOG
			bootinfo.power_off_reason_counter[i] = pooc.counter[i];
		}
		fclose(fp);
	}
	else
	{
		ALOGE("open RSV failed");
	}
#endif // #ifdef CCI_ENGMODE_WORKAROUND

	ioctl(dev_fd, KLOG_IOCTL_GET_BOOTINFO, &bootinfo);

#ifdef CCI_ENGMODE_LOG
	ALOGI("emergency_download_mode=%d", bootinfo.emergency_download_mode);
	ALOGI("qct_download_mode=%d", bootinfo.qct_download_mode);
	ALOGI("cci_download_mode=%d", bootinfo.cci_download_mode);
	ALOGI("mass_storage_download_mode=%d", bootinfo.mass_storage_download_mode);
	ALOGI("sd_download_mode=%d", bootinfo.sd_download_mode);
	ALOGI("usb_ram_dump_mode=%d", bootinfo.usb_ram_dump_mode);
	ALOGI("sd_ram_dump_mode=%d", bootinfo.sd_ram_dump_mode);
	ALOGI("normal_boot_mode=%d", bootinfo.normal_boot_mode);
	ALOGI("sd_boot_mode=%d", bootinfo.sd_boot_mode);
	ALOGI("fastboot_mode=%d", bootinfo.fastboot_mode);
	ALOGI("recovery_mode=%d", bootinfo.recovery_mode);
	ALOGI("simple_test_mode=%d", bootinfo.simple_test_mode);
	ALOGI("charging_only_mode=%d", bootinfo.charging_only_mode);
	ALOGI("android_safe_mode=%d", bootinfo.android_safe_mode);
	ALOGI("qct_secure_boot_mode=%d", bootinfo.qct_secure_boot_mode);
	ALOGI("cci_secure_boot_mode=%d", bootinfo.cci_secure_boot_mode);
	for(i = 0; i < POWER_REASON_RECENT_LENGTH; i++)
	{
		ALOGI("power_on_reason_recent[%d]=0x%X", i, bootinfo.power_on_reason_recent[i]);
	}
	for(i = 0; i < POWER_REASON_COUNTER_LENGTH; i++)
	{
		ALOGI("power_on_reason_counter[%d]=0x%X", i, bootinfo.power_on_reason_counter[i]);
	}
	for(i = 0; i < POWER_REASON_RECENT_LENGTH; i++)
	{
		ALOGI("power_off_reason_recent[%d]=0x%X", i, bootinfo.power_off_reason_recent[i]);
	}
	for(i = 0; i < POWER_REASON_COUNTER_LENGTH; i++)
	{
		ALOGI("power_off_reason_counter[%d]=0x%X", i, bootinfo.power_off_reason_counter[i]);
	}
	for(i = 0; i < POWER_REASON_COUNTER_LENGTH; i++)
	{
		ALOGI("crash_record[%d]=0x%X", i, bootinfo.crash_record[i]);
	}
#endif // #ifdef CCI_ENGMODE_LOG
}

void get_device_information(int dev_fd)
{
	struct device_information deviceinfo;

	char modem_version[KLOG_MODEM_VERSION_LENGTH] = {0};
	FILE *fp = NULL;
	char line[1024] = {0};
	char major[10] = {0};
	char minor[10] = {0};
	char blocks[10] = {0};
	char name[10] = {0};

	memset(&deviceinfo, 0, sizeof(struct device_information));

	fp = fopen("/proc/partitions", "r");
	if(fp)
	{
		while(fgets(line, sizeof(line), fp))
		{
			line[strlen(line) - 1] = '\0';
			sscanf(line, "%10s %10s %10s %10s\n", major, minor, blocks, name);
			if(strcmp(name, "mmcblk0") == 0)
			{
				deviceinfo.emmc = atoi(blocks);
			}
		}
		fclose(fp);
	}
	property_get("gsm.version.baseband", modem_version, "-");
	strncpy(deviceinfo.modem_version, modem_version, KLOG_MODEM_VERSION_LENGTH - 1);

	ioctl(dev_fd, KLOG_IOCTL_GET_DEVICEINFO, &deviceinfo);

#ifdef CCI_ENGMODE_LOG
	ALOGI("hw_id=%d", deviceinfo.hw_id);
	ALOGI("band_id=%d", deviceinfo.band_id);
	ALOGI("model_id=%d", deviceinfo.model_id);
	ALOGI("nfc_id=%d", deviceinfo.nfc_id);
	ALOGI("main_chip=%s", deviceinfo.main_chip);
	ALOGI("radio_chip=%s", deviceinfo.radio_chip);
	ALOGI("cpuinfo_max_freq=%u", deviceinfo.cpuinfo_max_freq);
	ALOGI("scaling_max_freq=%u", deviceinfo.scaling_max_freq);
	ALOGI("ram=%u", deviceinfo.ram);
	ALOGI("flash=%u", deviceinfo.flash);
	ALOGI("emmc=%u", deviceinfo.emmc);
	ALOGI("hw_reset=%d", deviceinfo.hw_reset);
	ALOGI("android_version=%s", deviceinfo.android_version);
	ALOGI("modem_version=%s", deviceinfo.modem_version);
	ALOGI("flex_version=%s", deviceinfo.flex_version);
	ALOGI("rpm_version=%s", deviceinfo.rpm_version);
	ALOGI("bootloader_version=%s", deviceinfo.bootloader_version);
	ALOGI("linux_version=%s", deviceinfo.linux_version);
	ALOGI("version_id=%s", deviceinfo.version_id);
	ALOGI("build_version=%s", deviceinfo.build_version);
	ALOGI("build_date=%s", deviceinfo.build_date);
	ALOGI("build_type=%s", deviceinfo.build_type);
	ALOGI("build_user=%s", deviceinfo.build_user);
	ALOGI("build_host=%s", deviceinfo.build_host);
	ALOGI("build_key=%s", deviceinfo.build_key);
	ALOGI("secure_mode=%d", deviceinfo.secure_mode);
#endif // #ifdef CCI_ENGMODE_LOG
}

void get_debug_information(int dev_fd)
{
	struct debug_information debuginfo;

	memset(&debuginfo, 0, sizeof(struct debug_information));

	ioctl(dev_fd, KLOG_IOCTL_GET_DEBUGINFO, &debuginfo);

#ifdef CCI_ENGMODE_LOG
	ALOGI("debug_mode=%d", debuginfo.debug_mode);
	ALOGI("usb_sbl_diag=%d", debuginfo.usb_sbl_diag);
	ALOGI("usb_diag=%d", debuginfo.usb_diag);
	ALOGI("usb_adb=%d", debuginfo.usb_adb);
	ALOGI("uart=%d", debuginfo.uart);
	ALOGI("jtag=%d", debuginfo.jtag);
	ALOGI("hot_key_ram_dump=%d", debuginfo.hot_key_ram_dump);
	ALOGI("command_ram_dump=%d", debuginfo.command_ram_dump);
	ALOGI("crash_ram_dump=%d", debuginfo.crash_ram_dump);
	ALOGI("bootloader_log=%d", debuginfo.bootloader_log);
	ALOGI("kernel_log=%d", debuginfo.kernel_log);
	ALOGI("logcat_log=%d", debuginfo.logcat_log);
	ALOGI("klog_log=%d", debuginfo.klog_log);
	ALOGI("rpm_log=%d", debuginfo.rpm_log);
#endif // #ifdef CCI_ENGMODE_LOG
}

void get_performance_information(int dev_fd)
{
	struct performance_information performanceinfo;

	FILE *fp = NULL;
	struct timespec ts;
	struct timespec up_timespec;
	int elapsed = 0;
	float up_time = 0;
	float idle_time = 0;
	float sleep_time = 0;
	int fd = 0;
	int result = 0;

	memset(&performanceinfo, 0, sizeof(struct performance_information));

	fp = fopen("/proc/uptime", "r");
	if(fp)
	{
		if(fscanf(fp, "%*f %f", &idle_time) != 1)
		{
			ALOGE("Could not parse /proc/uptime");
		}
		else
		{
			if(clock_gettime(CLOCK_MONOTONIC, &up_timespec) < 0)
			{
				ALOGE("Could not get monotonic time");
			}
			else
			{
				up_time = up_timespec.tv_sec + up_timespec.tv_nsec / 1e9;

				fd = open("/dev/alarm", O_RDONLY);
				if(fd < 0)
				{
					result = -1;
				}
				else
				{
					result = ioctl(fd, ANDROID_ALARM_GET_TIME(ANDROID_ALARM_ELAPSED_REALTIME), &ts);
					close(fd);
				}
				if (result == 0)
				{
					elapsed = ts.tv_sec;
				}
				else
				{
					elapsed = -1;
				}
				if(elapsed < 0)
				{
					ALOGE("elapsedRealtime failed");
				}
				else
				{
					ALOGI("elapsed=%d, idle_time=%f, up_time=%f", elapsed, idle_time, up_time);
					performanceinfo.up_time = elapsed * 1000;
					performanceinfo.idle_time = (int)idle_time * 1000;
					performanceinfo.sleep_time = (int)(elapsed - up_time) * 1000;
				}
			}
		}
		fclose(fp);
	}

	ioctl(dev_fd, KLOG_IOCTL_GET_PERFORMANCEINFO, &performanceinfo);

#ifdef CCI_ENGMODE_LOG
	ALOGI("bootup_time=%u", performanceinfo.bootup_time);
	ALOGI("shutdown_time=%u", performanceinfo.shutdown_time);
	ALOGI("suspend_time=%u", performanceinfo.suspend_time);
	ALOGI("resume_time=%u", performanceinfo.resume_time);
	ALOGI("up_time=%u", performanceinfo.up_time);
	ALOGI("idle_time=%u", performanceinfo.idle_time);
	ALOGI("sleep_time=%u", performanceinfo.sleep_time);
#endif // #ifdef CCI_ENGMODE_LOG

	property_set("sys.engmode.performance", "0");
}

void get_warranty_information(int dev_fd)
{
	struct warranty_information warrantyinfo;

#ifdef CCI_ENGMODE_WORKAROUND
	FILE* fp = NULL;
#endif // #ifdef CCI_ENGMODE_WORKAROUND

	memset(&warrantyinfo, 0, sizeof(struct warranty_information));

#ifdef CCI_ENGMODE_WORKAROUND
	fp = fopen("/dev/block/mmcblk0p5", "rb");
	if(fp != NULL)
	{
		fseek(fp, RSV_WARRANTY_ADDR + sizeof(unsigned int), SEEK_SET);//0x20th-sector and offset int
		fread((void*)warrantyinfo.total_deviceon_time, sizeof(warrantyinfo.total_deviceon_time), 1, fp);

		fseek(fp, RSV_WARRANTY_ADDR + sizeof(warrantyinfo.total_deviceon_time) + sizeof(unsigned int), SEEK_SET);//0x20th-sector and offset int
		fread((void*)warrantyinfo.first_voice_call_timestamp, sizeof(warrantyinfo.first_voice_call_timestamp), 1, fp);

		fseek(fp, RSV_WARRANTY_ADDR + sizeof(warrantyinfo.total_deviceon_time) + sizeof(warrantyinfo.first_voice_call_timestamp) + sizeof(unsigned int), SEEK_SET);//0x20th-sector and offset int
		fread((void*)warrantyinfo.first_data_call_timestamp, sizeof(warrantyinfo.first_data_call_timestamp), 1, fp);

#ifdef CCI_ENGMODE_LOG
//uint : ms
		ALOGI("total_deviceon_time=%s", warrantyinfo.total_deviceon_time);
//format yyyy/mm/dd/hh/mm/ss
		ALOGI("first_voice_call_timestamp=%s", warrantyinfo.first_voice_call_timestamp);
//format yyyy/mm/dd/hh/mm/ss
		ALOGI("first_data_call_timestamp=%s", warrantyinfo.first_data_call_timestamp);
#endif // #ifdef CCI_ENGMODE_LOG
		fclose(fp);
	}
	else
	{
		ALOGE("process warranty fail");
	}
#endif // #ifdef CCI_ENGMODE_WORKAROUND

	ioctl(dev_fd, KLOG_IOCTL_GET_WARRANTYINFO, &warrantyinfo);

#ifdef CCI_ENGMODE_LOG
	ALOGI("total_deviceon_time=%s", warrantyinfo.total_deviceon_time);
	ALOGI("first_voice_call_timestamp=%s", warrantyinfo.first_voice_call_timestamp);
	ALOGI("first_data_call_timestamp=%s", warrantyinfo.first_data_call_timestamp);
	ALOGI("maintenance_record=%d", warrantyinfo.maintenance_record);
#endif // #ifdef CCI_ENGMODE_LOG
}

void get_factory_information(int dev_fd)
{
	struct factory_information factoryinfo;

#ifdef CCI_ENGMODE_WORKAROUND
	struct rsv_flag rflag, rflag_force;
	int sector_no;
	int cmd_backup = -1;
	char buf[100];
	char buf_sector_fsg[0x200];
	char buf_sector_rsvsec0[0x200];
	char buf_sector_rsvsec2[0x200];
	char retval = 0;
	unsigned char static_super_blk_data[16] = {0x9a, 0x47, 0xf4, 0x9f, 0x5d, 0x2d, 0x03, 0xa3, 0x58, 0x75, 0xa8, 0xe5, 0x9f, 0xe7, 0x79, 0x0a};
	FILE *fp = NULL;
	FILE *fp2 = NULL;
#endif // #ifdef CCI_ENGMODE_WORKAROUND

	memset(&factoryinfo, 0, sizeof(struct factory_information));

#ifdef CCI_ENGMODE_WORKAROUND
	fp = fopen("/dev/block/mmcblk0p5", "rb");
	fp2 = fopen("/dev/block/mmcblk0p6", "rb");
	factoryinfo.backup_record = 0;
	if((fp != NULL) && (fp2 != NULL))
	{
		sector_no = 0;
		fseek(fp, 0x200 * sector_no, SEEK_SET);//0x0th-sector
		fread((void*)buf_sector_rsvsec0, sizeof(buf_sector_rsvsec0), 1, fp);

		sector_no = 2;
		fseek(fp, 0x200 * sector_no, SEEK_SET);//0x2th-sector
		fread((void*)buf_sector_rsvsec2, sizeof(buf_sector_rsvsec2), 1, fp);

		sector_no = 0;
		fseek(fp2, 0x200 * sector_no, SEEK_SET);//0x0th-sector of fsg
		fread((void*)buf_sector_fsg, sizeof(buf_sector_fsg), 1, fp2);

		rflag.start_magic = RSV_COOKIE_START;
		rflag.index = 0;
		rflag.cookie_id = FS_COOKIE_ID_BACKUP;
		rflag.process = FS_PROCESS_END;
		rflag.status = BACKUP_RECOVER_SUCCESS;
		rflag.end_magic = RSV_COOKIE_END;

		rflag_force.start_magic = RSV_COOKIE_START;
		rflag_force.index = 0;
		rflag_force.cookie_id = FS_COOKIE_ID_FORCE_BACKUP_FLAG;
		rflag_force.process = FS_PROCESS_END;
		rflag_force.status = BACKUP_RECOVER_SUCCESS;
		rflag_force.end_magic = RSV_COOKIE_END;

		if((memcmp((void*) static_super_blk_data, (void*) buf_sector_fsg, sizeof(static_super_blk_data)) == 0) &&\
		  ((memcmp((void*) &rflag, (void*) buf_sector_rsvsec0, sizeof(rflag)) == 0) || (memcmp((void*) &rflag_force, (void*) buf_sector_rsvsec2, sizeof(rflag_force)) == 0)))
		{
			ALOGE("backup result!! (Backup success)");
			factoryinfo.backup_record = 1;
		}
		else if((memcmp((void*) static_super_blk_data, (void*) buf_sector_fsg, sizeof(static_super_blk_data)) != 0) &&\
		  ((memcmp((void*) &rflag, (void*) buf_sector_rsvsec0, sizeof(rflag)) == 0) || (memcmp((void*) &rflag_force, (void*) buf_sector_rsvsec2, sizeof(rflag_force)) == 0)))
		{
			cmd_backup = 1;
			factoryinfo.backup_record = 0;
			ALOGE("%d <= backup result!! (Static sb data fail)", cmd_backup);
		}
		else if((memcmp((void*) &rflag, (void*) buf_sector_rsvsec0, sizeof(rflag.start_magic)) == 0) ||\
		 (memcmp((void*) &rflag_force, (void*) buf_sector_rsvsec2, sizeof(rflag_force.start_magic)) == 0))
		{
			cmd_backup = 2;
			factoryinfo.backup_record = 0;
			ALOGE("%d <= backup result!! (Backup fail)", cmd_backup);
		}
		else
		{
			cmd_backup = 3;
			factoryinfo.backup_record = 0;
			ALOGE("%d <= backup result!! (Backup isn't called yet)", cmd_backup);
		}
#ifdef CCI_ENGMODE_LOG
		ALOGI("backup_record=%d", factoryinfo.backup_record);
#endif // #ifdef CCI_ENGMODE_LOG
		fclose(fp);
		fclose(fp2);
	}
#endif // #ifdef CCI_ENGMODE_WORKAROUND

	ioctl(dev_fd, KLOG_IOCTL_GET_FACTORYINFO, &factoryinfo);

#ifdef CCI_ENGMODE_LOG
	ALOGI("backup_command=%d", factoryinfo.backup_command);
	ALOGI("restore_command=%d", factoryinfo.restore_command);
	ALOGI("backup_record=%d", factoryinfo.backup_record);
#endif // #ifdef CCI_ENGMODE_LOG
}

unsigned long long emmc_test_write(char seed, unsigned long long size, unsigned int unit, char cmp)
{
	struct timespec start_time;
	struct timespec end_time;
	unsigned long long counter = 0;
	unsigned long long period = 0;
	unsigned long long speed = 0;
	char *buffer = NULL;
	int fd = 0;
	int ret = 0;

	counter = 1024 * 1024 * size / unit;
	buffer = (char *)memalign(unit, unit);
	memset(buffer, seed, unit);

	fd = open("/sdcard/test_emmc", O_CREAT | O_WRONLY | O_TRUNC | O_DIRECT, 0600);

	if(fd >= 0)
	{
		ALOGI("write emmc test counter:%llu, unit:%u", counter, unit);

		if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0)
		{
			ALOGE("Could not get start time");
			speed = -1;
			goto finish_emmc_test_write;
		}
		while(counter > 0)
		{
			ret = write(fd, buffer, unit);
			if(ret >= 0)
			{
				counter--;
			}
			else
			{
				ALOGE("write emmc error!count:%llu, seed:%d, ret:%d", (1024 * 1024 * size / unit - counter), seed, ret);
				speed = -1;
				goto finish_emmc_test_write;
			}
		}

		while(fsync(fd) < 0)
		{
			ALOGE("fsync failed");
			sleep(1);
		}

		if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0)
		{
			ALOGE("Could not get end time");
			speed = -1;
			goto finish_emmc_test_write;
		}

		ALOGI("write emmc start time:%ld.%ld", start_time.tv_sec, start_time.tv_nsec);
		ALOGI("write emmc end time:%ld.%ld", end_time.tv_sec, end_time.tv_nsec);
		size = 1024 * 1024 * size;//byte
		period = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_nsec - start_time.tv_nsec) / 1000000;//millisecond
		if(period > 0)
		{
			speed = (size * 1000) / (period * 1024);
			ALOGI("write emmc performance:%llu KB/s", speed);
		}
		else
		{
			speed = 0;
		}
	}
	else
	{
		speed = -1;
	}

finish_emmc_test_write:
	if(fd >= 0)
	{
		close(fd);
	}
	free(buffer);
	buffer = NULL;
	return speed;
}

unsigned long long emmc_test_read(char seed, unsigned long long size, unsigned int unit, char cmp)
{
	struct timespec start_time;
	struct timespec end_time;
	unsigned long long counter = 0;
	unsigned long long period = 0;
	unsigned long long speed = 0;
	char *buffer = NULL;
	char *buffer2 = NULL;
	int fd = 0;
	int ret = 0;

	counter = 1024 * 1024 * size / unit;
	buffer = (char *)memalign(unit, unit);
	buffer2 = (char *)memalign(unit, unit);
	memset(buffer, seed - 1, unit);
	memset(buffer2, seed, unit);

	fd = open("/sdcard/test_emmc", O_RDONLY | O_DIRECT);

	if(fd >= 0)
	{
		ALOGI("read emmc test counter:%llu, unit:%u", counter, unit);

		lseek(fd, 0, SEEK_SET);
		if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0)
		{
			ALOGE("Could not get start time");
			speed = -1;
			goto finish_emmc_test_read;
		}
		while(counter > 0)
		{
			ret = read(fd, buffer, unit);
			if(ret >= 0)
			{
				if(cmp == 1)
				{
					ret = memcmp(buffer, buffer2, unit);
				}
				else
				{
					ret = 0;
				}
				if(ret == 0)
				{
					counter--;
					memset(buffer, seed - 1, unit);
				}
				else
				{
					ALOGE("compare error!count:%llu, seed:%d, ret:%d", (1024 * 1024 * size / unit - counter), seed, ret);
					speed = -1;
					goto finish_emmc_test_read;
				}
			}
			else
			{
				ALOGE("read emmc error!count:%llu, seed:%d, ret:%d", (1024 * 1024 * size / unit - counter), seed, ret);
				break;
			}
		}
		if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0)
		{
			ALOGE("Could not get end time");
			speed = -1;
			goto finish_emmc_test_read;
		}

		ALOGI("read emmc start time:%ld.%ld", start_time.tv_sec, start_time.tv_nsec);
		ALOGI("read emmc end time:%ld.%ld", end_time.tv_sec, end_time.tv_nsec);
		size = 1024 * 1024 * size;//byte
		period = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_nsec - start_time.tv_nsec) / 1000000;//millisecond
		if(period > 0)
		{
			speed = (size * 1000) / (period * 1024);
			ALOGI("read emmc performance:%llu KB/s", speed);
		}
		else
		{
			speed = 0;
		}
	}
	else
	{
		speed = -1;
	}

finish_emmc_test_read:
	if(fd >= 0)
	{
		close(fd);
	}
	free(buffer);
	free(buffer2);
	buffer = NULL;
	buffer2 = NULL;
	return speed;
}

unsigned long long ram_test_write(char seed, unsigned long long size, unsigned int unit, char cmp)
{
	struct timespec start_time;
	struct timespec end_time;
	unsigned long long counter = 0;
	unsigned long long period = 0;
	unsigned long long speed = 0;
	char *buffer = NULL;

	counter = 1024 * 1024 * size;
	buffer = (char *)malloc(counter);

	ALOGI("write ram test counter:%llu, unit:%u", counter, unit);

	if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0)
	{
		ALOGE("Could not get start time");
		speed = -1;
		goto finish_ram_test_write;
	}

	memset(buffer, seed, counter);

	if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0)
	{
		ALOGE("Could not get end time");
		speed = -1;
		goto finish_ram_test_write;
	}

	ALOGI("write ram start time:%ld.%ld", start_time.tv_sec, start_time.tv_nsec);
	ALOGI("write ram end time:%ld.%ld", end_time.tv_sec, end_time.tv_nsec);
	size = 1024 * 1024 * size;//byte
	period = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_nsec - start_time.tv_nsec) / 1000000;//millisecond
	if(period > 0)
	{
		speed = (size * 1000) / (period * 1024);
		ALOGI("write ram performance:%llu KB/s", speed);
	}
	else
	{
		speed = 0;
	}

finish_ram_test_write:
	free(buffer);
	buffer = NULL;
	return speed;
}

unsigned long long ram_test_read(char seed, unsigned long long size, unsigned int unit, char cmp)
{
	struct timespec start_time;
	struct timespec end_time;
	unsigned long long counter = 0;
	unsigned long long period = 0;
	unsigned long long speed = 0;
	char *buffer = NULL;
	char *buffer2 = NULL;
	int ret = 0;

	counter = 1024 * 1024 * size;
	buffer = (char *)malloc(counter);
	buffer2 = (char *)malloc(counter);
	memset(buffer, seed - 1, counter);
	memset(buffer2, seed, counter);

	ALOGI("read ram test counter:%llu, unit:%u", counter, unit);

	if(clock_gettime(CLOCK_MONOTONIC, &start_time) < 0)
	{
		ALOGE("Could not get start time");
		speed = -1;
		goto finish_ram_test_read;
	}

	memcpy(buffer, buffer2, counter);

	if(cmp == 1)
	{
		ret = memcmp(buffer, buffer2, unit);
		if(ret != 0)
		{
			ALOGE("compare error!seed:%d, ret:%d", seed, ret);
			speed = -1;
			goto finish_ram_test_read;
		}
	}

	if(clock_gettime(CLOCK_MONOTONIC, &end_time) < 0)
	{
		ALOGE("Could not get end time");
		speed = -1;
		goto finish_ram_test_read;
	}

	ALOGI("read ram start time:%ld.%ld", start_time.tv_sec, start_time.tv_nsec);
	ALOGI("read ram end time:%ld.%ld", end_time.tv_sec, end_time.tv_nsec);
	size = 1024 * 1024 * size;//byte
	period = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_nsec - start_time.tv_nsec) / 1000000;//millisecond
	if(period > 0)
	{
		speed = (size * 1000) / (period * 1024);
		ALOGI("read ram performance:%llu KB/s", speed);
	}
	else
	{
		speed = 0;
	}

finish_ram_test_read:
	free(buffer);
	free(buffer2);
	buffer = NULL;
	buffer2 = NULL;
	return speed;
}

int main(int argc, char **argv)
{
	char emmc_test_seed = 0;
	char emmc_test_cmp = 0;
	char *buffer = NULL;
	char length = 0;
	unsigned long long speed = 0;
	unsigned long long emmc_test_write_speed = 0;
	unsigned long long emmc_test_read_speed = 0;
	unsigned long long emmc_test_size = 0;
	unsigned int emmc_test_unit = 0;
	int emmc_test_counter = 0;
	int emmc_test_wait = 0;

	int i = 0;

	if(argc > 1)
	{
		if(argv[1] && strlen(argv[1]) > 0)
		{
			int dev_fd = open(KLOG_DEV_PATH, O_RDWR);

			if(dev_fd == -1)
			{
				ALOGE("Can't open %s", KLOG_DEV_PATH);
				printf("Can't open %s\n", KLOG_DEV_PATH);

				return -1;
			}

			if(strcmp(argv[1], "boot") == 0)
			{
				get_boot_information(dev_fd);
			}
			else if(strcmp(argv[1], "device") == 0)
			{
				get_device_information(dev_fd);
			}
			else if(strcmp(argv[1], "debug") == 0)
			{
				get_debug_information(dev_fd);
			}
			else if(strcmp(argv[1], "performance") == 0)
			{
				get_performance_information(dev_fd);
			}
			else if(strcmp(argv[1], "warranty") == 0)
			{
				get_warranty_information(dev_fd);
			}
			else if(strcmp(argv[1], "factory") == 0)
			{
				get_factory_information(dev_fd);
			}
			else if(strcmp(argv[1], "all") == 0)
			{
				get_boot_information(dev_fd);
				get_device_information(dev_fd);
				get_debug_information(dev_fd);
				get_performance_information(dev_fd);
				get_warranty_information(dev_fd);
				get_factory_information(dev_fd);
			}
			else if(strcmp(argv[1], "emmctest") == 0)
			{
				i = 2;
				while(argv[i])
				{
					length = strlen(argv[i]);
					buffer = malloc(length + 1);
					memset(buffer, 0, length + 1);
					strncpy(buffer, argv[i], length);
					if(strncmp(buffer, "seed:", 5) == 0)
					{
						emmc_test_seed = buffer[5];
					}
					if(strncmp(buffer, "unit:", 5) == 0)
					{
						emmc_test_unit = atoi(&buffer[5]);
					}
					if(strncmp(buffer, "size:", 5) == 0)
					{
						emmc_test_size = atoi(&buffer[5]);
					}
					if(strncmp(buffer, "cmp", 3) == 0)
					{
						emmc_test_cmp = 1;
					}
					if(strncmp(buffer, "counter:", 8) == 0)
					{
						emmc_test_counter = atoi(&buffer[8]);
					}
					if(strncmp(buffer, "wait:", 5) == 0)
					{
						emmc_test_wait = atoi(&buffer[5]);
					}
					free(buffer);
					i++;
				}

				srand(time(NULL));
				if(emmc_test_seed == 0)
				{
					emmc_test_seed = rand() % 26 + 0x41 + 0x20 * (rand() % 2);//alphabet
				}
				if(emmc_test_unit == 0)
				{
					emmc_test_unit = 4194304;//sector:512, page:4096, ext4 block:4194304
				}
				if(emmc_test_size == 0)
				{
					emmc_test_size = 200;//(rand() % 50 + 1) * 4;//4~200MB, 4MB alignment
				}
				if(emmc_test_counter <= 0)
				{
					emmc_test_counter = 1;
				}
				if(emmc_test_wait <= 0)
				{
					emmc_test_wait = 1;
				}
				ALOGI("test emmc:seed=%d(%c), unit=%lu, size=%llu, cmp=%d, counter=%d, wait=%d", emmc_test_seed, emmc_test_seed, emmc_test_unit, emmc_test_size, emmc_test_cmp, emmc_test_counter, emmc_test_wait);
				for(i = 1; i <= emmc_test_counter; i++)
				{
					speed = emmc_test_write(emmc_test_seed, emmc_test_size, emmc_test_unit, emmc_test_cmp);
					if(speed <= 0)
					{
						ALOGI("emmc write test failed");
						break;
					}
					if(emmc_test_write_speed)
					{
						emmc_test_write_speed = (emmc_test_write_speed + speed) / 2;
					}
					else
					{
						emmc_test_write_speed = speed;
					}

					speed = emmc_test_read(emmc_test_seed, emmc_test_size, emmc_test_unit, emmc_test_cmp);
					if(speed <= 0)
					{
						ALOGI("emmc read test failed");
						break;
					}
					if(emmc_test_read_speed)
					{
						emmc_test_read_speed = (emmc_test_read_speed + speed) / 2;
					}
					else
					{
						emmc_test_read_speed = speed;
					}

					ALOGI("test emmc success:%d, average speed:write %llu KB/s, read %llu KB/s", i, emmc_test_write_speed, emmc_test_read_speed);
					if(i != emmc_test_counter)
					{
						sleep(emmc_test_wait);
					}
				}
			}
			else if(strcmp(argv[1], "ramtest") == 0)
			{
				i = 2;
				while(argv[i])
				{
					length = strlen(argv[i]);
					buffer = malloc(length + 1);
					memset(buffer, 0, length + 1);
					strncpy(buffer, argv[i], length);
					if(strncmp(buffer, "seed:", 5) == 0)
					{
						emmc_test_seed = buffer[5];
					}
					if(strncmp(buffer, "unit:", 5) == 0)
					{
						emmc_test_unit = atoi(&buffer[5]);
					}
					if(strncmp(buffer, "size:", 5) == 0)
					{
						emmc_test_size = atoi(&buffer[5]);
					}
					if(strncmp(buffer, "cmp", 3) == 0)
					{
						emmc_test_cmp = 1;
					}
					if(strncmp(buffer, "counter:", 8) == 0)
					{
						emmc_test_counter = atoi(&buffer[8]);
					}
					if(strncmp(buffer, "wait:", 5) == 0)
					{
						emmc_test_wait = atoi(&buffer[5]);
					}
					free(buffer);
					i++;
				}

				srand(time(NULL));
				if(emmc_test_seed == 0)
				{
					emmc_test_seed = rand() % 26 + 0x41 + 0x20 * (rand() % 2);//alphabet
				}
				if(emmc_test_unit == 0)
				{
					emmc_test_unit = 4194304;//sector:512, page:4096, ext4 block:4194304
				}
				if(emmc_test_size == 0)
				{
					emmc_test_size = 20;//(rand() % 50 + 1) * 4;//4~200MB, 4MB alignment
				}
				if(emmc_test_counter <= 0)
				{
					emmc_test_counter = 1;
				}
				if(emmc_test_wait <= 0)
				{
					emmc_test_wait = 1;
				}
				ALOGI("test ram:seed=%d(%c), unit=%lu, size=%llu, cmp=%d, counter=%d, wait=%d", emmc_test_seed, emmc_test_seed, emmc_test_unit, emmc_test_size, emmc_test_cmp, emmc_test_counter, emmc_test_wait);
				for(i = 1; i <= emmc_test_counter; i++)
				{
					speed = ram_test_write(emmc_test_seed, emmc_test_size, emmc_test_unit, emmc_test_cmp);
					if(speed <= 0)
					{
						ALOGI("ram write test failed");
						break;
					}
					if(emmc_test_write_speed)
					{
						emmc_test_write_speed = (emmc_test_write_speed + speed) / 2;
					}
					else
					{
						emmc_test_write_speed = speed;
					}

					speed = ram_test_read(emmc_test_seed, emmc_test_size, emmc_test_unit, emmc_test_cmp);
					if(speed <= 0)
					{
						ALOGI("ram read test failed");
						break;
					}
					if(emmc_test_read_speed)
					{
						emmc_test_read_speed = (emmc_test_read_speed + speed) / 2;
					}
					else
					{
						emmc_test_read_speed = speed;
					}

					ALOGI("test ram success:%d, average speed:write %llu KB/s, read %llu KB/s", i, emmc_test_write_speed, emmc_test_read_speed);
					if(i != emmc_test_counter)
					{
						sleep(emmc_test_wait);
					}
				}
			}
			close(dev_fd);
		}
	}

	return 0;
}

