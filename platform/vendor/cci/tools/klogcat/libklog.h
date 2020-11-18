#ifndef __LIBKLOG_H__
#define __LIBKLOG_H__

#ifdef __cplusplus
extern "C" {
#endif
//klog_storage_mount_status
struct KSMS
{
	int UserdataMounted;
	int InternalMounted;
	int ExternalMounted;
};

int do_klogcat(int fd, int dump_crash);
int dump_to_target(const char* target_path, int dump_crash);
int record_sysinfo(int system_on, int system_state);
struct KSMS checkStorageMountStatus(void);
int copy_file(const char* source, const char* target);
int get_pid_name(pid_t pid, char* name);
void get_process_info(struct klog_category* pklog_category);
int perform_klog_ioctl(unsigned int ioctl_code, void* arg);
int check_old_log(void);

#ifdef __cplusplus
}
#endif

#endif

