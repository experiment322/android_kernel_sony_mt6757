#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <ctype.h>

#define LOG_TAG "libklog"
#include <utils/Log.h>
#include "cciklog.h"
#include "libklog.h"

static __inline__ void __klog_append_char(unsigned char c, struct klog_category* pklog_category)
{
	pklog_category->buffer[pklog_category->index++] = c;
	if(pklog_category->index >= pklog_category->size - KLOG_CATEGORY_HEADER_SIZE)
	{
		pklog_category->overload = 1;
		pklog_category->index = 0;
	}
}

static __inline__ void __klog_append_str(unsigned char *str, int len, struct klog_category* pklog_category)
{
	int idx = 0;

	for(idx = 0; idx < len; idx++)
	{
		__klog_append_char(*(str + idx), pklog_category);
	}

	return;
}

//Get next token via finding the givien seperator
static char *nxtoken(char **strp, char *sep)
{
	char *p = strsep(strp,sep);

	return (p == 0) ? "" : p;
}

//Skip multiple tokens that we don't need
static void skiptokens(char **strp, char *sep, int num)
{
	int t = 0;

	for(t = 0; t < num; t++)
	{
		nxtoken(strp, sep);
	}

	return;
}

static int read_each_process(int pid, int tid, struct klog_category* pklog_category)
{
	char statline[1024] = {0};
	char cmdline[1024] = {0};
	char user[32] = {0};
	char prBuff[512] = {0};
	char *ptr, *name, *state;
	struct stat stats;
	int fd, r, strlen;
	int ppid, tty, prio, nice, rtprio, sched;
	unsigned wchan, rss, vss, eip;
	unsigned utime, stime;
	struct passwd *pw;

	snprintf(statline, 18, "/proc/%d", pid);//"/proc/" + 11-digit (-2147483647 ~ 2147483648) + '\0'
	if(stat(statline, &stats) != 0)
	{
		return KLOG_ERROR;
	}

	snprintf(statline, 23, "/proc/%d/stat", pid);//"/proc/" + 11-digit (-2147483647 ~ 2147483648) + "/stat" + '\0'

//get process command line name
	snprintf(cmdline, 26, "/proc/%d/cmdline", pid);//"/proc/" + 11-digit (-2147483647 ~ 2147483648) + "/cmdline" + '\0'
	fd = open(cmdline, O_RDONLY);
	if(fd == 0)
	{
		r = 0;
	}
	else
	{
		r = read(fd, cmdline, 1023);
		close(fd);
		if(r < 0)
		{
			r = 0;
		}
	}
	cmdline[r] = 0;

//get process status info
	fd = open(statline, O_RDONLY);
	if(fd == 0)
	{
		return KLOG_ERROR;
	}
	r = read(fd, statline, 1023);
	close(fd);
	if(r < 0)
	{
		return KLOG_ERROR;
	}
	statline[r] = 0;

	ptr = statline;
	nxtoken(&ptr, " ");// skip pid
	ptr++;// skip "("

	name = ptr;
	ptr = strrchr(ptr, ')');// Skip to *last* occurence of ')',
	*ptr++ = '\0';// and null-terminate name.

	ptr++;// skip " "
	state = nxtoken(&ptr, " ");
	ppid = atoi(nxtoken(&ptr, " "));
	nxtoken(&ptr, " ");// pgrp
	nxtoken(&ptr," ");// sid
	tty = atoi(nxtoken(&ptr, " "));

	skiptokens(&ptr, " ", 6);// Skip tpgid, flags, minflt, cminflt, majflt and cmajflt

	utime = atoi(nxtoken(&ptr, " "));
	stime = atoi(nxtoken(&ptr, " "));

	skiptokens(&ptr, " ", 2);// Skip cutime and cstime

	prio = atoi(nxtoken(&ptr, " "));
	nice = atoi(nxtoken(&ptr, " "));

	skiptokens(&ptr, " ", 3);// Skip threads, itrealvalue and starttime

	vss = strtoul(nxtoken(&ptr, " "), 0, 10);// vsize
	rss = strtoul(nxtoken(&ptr, " "), 0, 10);// rss

	skiptokens(&ptr, " ", 5);// Skip rlim, startcode, endcode, startstack, and kstkesp

	eip = strtoul(nxtoken(&ptr, " "), 0, 10);// kstkeip

	skiptokens(&ptr, " ", 4);// Skip signal, blocked, sigignore and sigcatch

	wchan = strtoul(nxtoken(&ptr, " "), 0, 10);// wchan

	skiptokens(&ptr, " ", 4);// Skip nswap, cnswap, exit signal, and processor

	rtprio = atoi(nxtoken(&ptr, " "));// rt_priority
	sched = atoi(nxtoken(&ptr, " "));// scheduling policy
	tty = atoi(nxtoken(&ptr, " "));

	if(tid != 0)
	{
		ppid = pid;
		pid = tid;
	}

	pw = getpwuid(stats.st_uid);
	if(pw == 0)
	{
		snprintf(user, 12, "%d", (int)stats.st_uid);//11-digit (-2147483647 ~ 2147483648) + '\0'
	}
	else
	{
		strncpy(user, pw->pw_name, 31);
	}

//print process information
	strlen = snprintf(prBuff, 60, "%-10s %-5d %-5d %-6d %-5d", user, pid, ppid, vss / 1024, rss * 4);//11-digit (-2147483647 ~ 2147483648) * 5 + " " * 4 + '\0'
	strlen += snprintf(prBuff + strlen, 49, " %-5d %-5d %-5d %-5d", prio, nice, rtprio, sched);//11-digit (-2147483647 ~ 2147483648) * 4 + " " * 4 + '\0'
	strlen += snprintf(prBuff + strlen, 511 - strlen - 32 - 2 - 1, " %08x %08x %s %s", wchan, eip, state, cmdline[0] ? cmdline : name);//max allowed size
	strlen += snprintf(prBuff + strlen, 32, " (u:%d, s:%d)", utime, stime);//" (u:" + ", s:" + ")" + 11-digit (-2147483647 ~ 2147483648) * 2 + '\0'
	strlen += snprintf(prBuff + strlen, 2, "\n");//'\n' + '\0'
	__klog_append_str(prBuff, strlen, pklog_category);
//	printf("%-4d %s",chCounts, prBuff );

	return KLOG_OK;
}

void get_process_info(struct klog_category* pklog_category)
{
	DIR *d;
	struct dirent *de;
	char prBuff[128] = {0};
	int strlen;

//init buffer
	memset(pklog_category, 0, sizeof(struct klog_category) + get_max_category_size() - 1);
	snprintf(pklog_category->name, KLOG_CATEGORY_NAME_LENGTH, KLOG_CATEGORY_NAME_PROCESS);

//get process info form /proc
	d = opendir("/proc");
	if(d == 0)
	{
		return;
	}

//column names
	strlen = snprintf(prBuff, 127, "user       PID   PPID  VSIZE  RSS   PRIO  NICE  RTPRI SCHED WCHAN    PC         NAME\n");
	__klog_append_str(prBuff, strlen, pklog_category);// Append this string to klog queue

//loop to get snapshots of all processes under /proc
	while((de = readdir(d)) != 0)
	{
		if(isdigit(de->d_name[0]))
		{
			int pid = atoi(de->d_name);
			if(read_each_process(pid, 0, pklog_category) == -1)
			{
				ALOGE("read process information failed:pid=%d", pid);
			}
		}
	}
	closedir(d);

	return;
}

