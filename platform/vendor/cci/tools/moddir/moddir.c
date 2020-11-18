#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include <android/log.h>
#include <string.h>
#include <sys/stat.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , "MODDIR",__VA_ARGS__)
#define TAG "MODDIR"

int main(int argc, char *argv[])
{
	FILE *dp;
	struct stat64 st;

	LOGI( "MODDIR: start\n");
	if((dp=fopen("/data/misc/mcRegistry/00000000.authtokcont.backup","w+"))==NULL)
	{
		fclose(dp);
		system("cp /data/misc/mcRegistry/00000000.authtokcont /data/misc/mcRegistry/00000000.authtokcont.backup");
		system("rm -rf /data/misc/mcRegistry/00000000.authtokcont");
	}
	else
	{
		fclose(dp);
		stat("/data/misc/mcRegistry/00000000.authtokcont.backup", &st);
		if(st.st_size == 0)
		{
			LOGI( "MODDIR: 00000000.authtokcont.backup size = 0. Copy from 00000000.authtokcont\n");
			system("rm -rf /data/misc/mcRegistry/00000000.authtokcont.backup");
			system("cp /data/misc/mcRegistry/00000000.authtokcont /data/misc/mcRegistry/00000000.authtokcont.backup");
			system("rm -rf /data/misc/mcRegistry/00000000.authtokcont");
		}
		else
			LOGI( "MODDIR: 00000000.authtokcont.backup already exists\n");
	}
	system("mkdir -p /data/misc/mcRegistry/TbStorage");
	system("chown system:system /data/misc/mcRegistry/*");
	system("chmod 0775 /data/misc/mcRegistry");
	system("chmod 0775 /data/misc/mcRegistry/*");
	system("chown -R system:system /data/misc/mcRegistry");
	system("chmod -R 0775 /data/misc/mcRegistry");
	LOGI( "MODDIR: end\n");

    return 0;
}

