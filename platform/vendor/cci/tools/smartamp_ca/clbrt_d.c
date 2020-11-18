#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cutils/log.h>
#include <errno.h> 

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "nxp_extamp_cal"

#undef  printf
#define  printf  ALOGE

// #define SIMULATION 1

#define CLIMAX_FIFO "/data/climax/climax_fifo"
#define CLIMAX_FIFO_W "/data/climax/climax_fifo_w"

const char *lpszKStart = "NXP KStart\n", *lpszKEnd = "NXP KEnd\n";

#ifdef SIMULATION
const char *lppszK00[] = {"/bin/ls", "-al", NULL};
#else
const char *lppszK00[] = {"/system/bin/tinymix", "Audio_i2s0_hd_Switch", "1", NULL};
const char *lppszK01[] = {"/system/bin/tinymix", "Audio_i2s0_SideGen_Switch", "5", NULL};
const char *lppszK02[] = {"/system/bin/tinymix", "AUD_CLK_BUF_Switch", "On", NULL};
const char *lppszK03[] = {"/system/bin/tinymix", "Audio IRQ1 CNT", "1024", NULL};
const char *lppszK04[] = {"/system/bin/tinyplay", "/system/etc/silence_sound.wav", "-D", "0", "-d", "7", NULL};
const char *lppszK05[] = {"/system/bin/climax", "-dsysfs", "-l", "/system/vendor/firmware/tfa98xx.cnt", "--start", NULL};
const char *lppszK06[] = {"/system/bin/climax", "-dsysfs", "-l", "/system/vendor/firmware/tfa98xx.cnt", "--resetMtpEx", NULL};
const char *lppszK07[] = {"/system/bin/climax", "-dsysfs", "-l", "/system/vendor/firmware/tfa98xx.cnt", "--reset", NULL};
const char *lppszK08[] = {"/system/bin/climax", "-dsysfs", "-l", "/system/vendor/firmware/tfa98xx.cnt", "--calibrate=once", NULL};
const char *lppszK09[] = {"/system/bin/climax", "-dsysfs", "-l", "/system/vendor/firmware/tfa98xx.cnt", "--calshow", NULL};
const char *lppszK10[] = {"/system/bin/climax", "-dsysfs", "--slave=0x36", "-r", "0x02", NULL};
const char *lppszK11[] = {"/system/bin/climax", "-dsysfs", "-l", "/system/vendor/firmware/tfa98xx.cnt", "--stop", NULL};
const char *lppszK12[] = {"/system/bin/tinymix", "Audio_i2s0_SideGen_Switch", "0", NULL};
const char *lppszK13[] = {"/system/bin/tinymix", "Audio_i2s0_hd_Switch", "0", NULL};
const char *lppszK14[] = {"/system/bin/cat", "/sys/class/power_supply/battery/batt_temp", NULL};
#endif
 
const char **lpppszKCommands[] =
{
	lppszK00,
#ifndef SIMULATION
	lppszK01, lppszK02, lppszK03, lppszK04,
	lppszK05, lppszK06,lppszK07, lppszK08,lppszK09, lppszK10,lppszK11,
	lppszK12,lppszK13,
	lppszK14,
#endif
	NULL
};

const char **lpppszKCommands_t[] =
{
	lppszK00,
#ifndef SIMULATION
	lppszK01, lppszK02, lppszK03,
	lppszK05, lppszK10,lppszK11,
	lppszK12,lppszK13,
	lppszK14,
#endif
	NULL
};

int execKProg(const char *lpszKProg, const char *lpszKParam[], int fdOut)
{
	int pid, status;

	if (0 == (pid = fork()))
	{
		dup2(fdOut, 1);
		dup2(fdOut, 2);
		close(fdOut);
		execv(lpszKProg, lpszKParam);
	}
	else if (pid > 0)
	{
		waitpid (pid, &status, 0);
	}
	return 0;
}

int main (int argc, char *argv[])
{
	int res, res_fifo, res_w, res_fifo_w;
	int climax_pipe, climax_pipe_w;
	//int chown_ret;
	char cCommand = 0;

	printf("claribation Enter\n");

	umask(000);
	if (access(CLIMAX_FIFO, F_OK) == -1)
        {
                res_fifo = mkfifo(CLIMAX_FIFO, 0770);
                if (res_fifo != 0)
                {
                        printf("can not create fifo : %s\n", CLIMAX_FIFO);
                        exit(EXIT_FAILURE);
                }
		printf("create fifo success: %s\n", CLIMAX_FIFO);
	//	chown_ret = chown(CLIMAX_FIFO, 0, 1000);
	//	if( chown_ret == -1 )
	//		printf("chown fifo(%s), ret:%d (%s) \n", CLIMAX_FIFO, chown_ret, strerror(errno));
	}
	else
		printf("fifo exist: %s\n", CLIMAX_FIFO);

	if (access(CLIMAX_FIFO_W, F_OK) == -1)
	{
		res_fifo_w = mkfifo(CLIMAX_FIFO_W, 0770);
		if (res_fifo_w != 0)
		{
			printf("can not create fifo_w : %s\n", CLIMAX_FIFO_W);
			exit(EXIT_FAILURE);
		}
		printf("create fifo_w success: %s\n", CLIMAX_FIFO_W);
	//	chown_ret = chown(CLIMAX_FIFO_W, 0, 1000);
	//	if( chown_ret == -1 )
	//		printf("chown fifo(%s), ret:%d (%s) \n", CLIMAX_FIFO, chown_ret, strerror(errno));
	}
	else
		printf("fifo exist: %s\n", CLIMAX_FIFO);

	printf("waiting for cal command ...\n");
	climax_pipe = open(CLIMAX_FIFO, O_RDONLY);
	if(climax_pipe == -1)
	{
		printf("open fifo(%s) failed (%s) \n", CLIMAX_FIFO, strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("open fifo success\n");
	res = read(climax_pipe, &cCommand, 1);
	printf("Command is %c\n", cCommand);
	close(climax_pipe);
	if ('k' != cCommand && 'K' != cCommand &&
		't' != cCommand && 'T' != cCommand)	// calibrate NXP smart amp;
	{
		printf("invalid command!\n");
		return 0;
	}

	printf("waiting for get result command ...\n");
	climax_pipe_w = open(CLIMAX_FIFO_W, O_WRONLY);
	if(climax_pipe_w == -1)
	{
		printf("open fifo(%s) failed (%s) \n", CLIMAX_FIFO_W, strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("open fifo_w success\n");

	if (climax_pipe_w != -1)
	{
		int i;
		const char **lppszKCommand;
		char lpszCommandMsg[100];

		if ('k' == cCommand || 'K' == cCommand)
		{
			printf("claribation start ...\n");
			res_w = write(climax_pipe_w, lpszKStart, strlen(lpszKStart)+1);
			for (i = 0; lppszKCommand = lpppszKCommands[i]; i++)
			{
				snprintf(lpszCommandMsg, sizeof(lpszCommandMsg)-1, "Executing K command %02d...\n", i);
				lpszCommandMsg[sizeof(lpszCommandMsg)-1] = 0;
				write(climax_pipe_w, lpszCommandMsg, strlen(lpszCommandMsg));

				execKProg(lppszKCommand[0], lppszKCommand, climax_pipe_w);
				usleep(200000);
			}
			res_w = write(climax_pipe_w, lpszKEnd, strlen(lpszKEnd)+1);
			printf("claribation end ...\n");
			usleep(1000000);
		}
		else if ('t' == cCommand || 'T' == cCommand)
		{
			printf("get temperature start ...\n");
			res_w = write(climax_pipe_w, lpszKStart, strlen(lpszKStart)+1);
			for (i = 0; lppszKCommand = lpppszKCommands_t[i]; i++)
			{
				snprintf(lpszCommandMsg, sizeof(lpszCommandMsg)-1, "Executing K command %02d...\n", i);
				lpszCommandMsg[sizeof(lpszCommandMsg)-1] = 0;
				write(climax_pipe_w, lpszCommandMsg, strlen(lpszCommandMsg));

				execKProg(lppszKCommand[0], lppszKCommand, climax_pipe_w);
				usleep(200000);
			}
			res_w = write(climax_pipe_w, lpszKEnd, strlen(lpszKEnd)+1);
			printf("get temperature end ...\n");
			usleep(1000000);
		}
	}

	close(climax_pipe_w);

	// if (res_fifo != -1)
	//	unlink(CLIMAX_FIFO);
	// if (res_fifo_w != -1)
	//	unlink(CLIMAX_FIFO_W);

	printf("claribation exit\n");
	return 0;
}
