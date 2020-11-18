#include "cciklog.h"

//boot information
#define RSV_POWER_ON_OFF_REASON_ADDR		(0x200 * 0x12)	//1 sector = 512 bytes
#define RSV_POWER_ON_REASON_RECENT_OFFSET	0
#define RSV_POWER_ON_REASON_COUNTER_OFFSET	0x50
#define RSV_POWER_OFF_REASON_RECENT_OFFSET	0x100
#define RSV_POWER_OFF_REASON_COUNTER_OFFSET	0x150

struct power_on_off_history
{
	int index;
	unsigned int reason[POWER_REASON_RECENT_LENGTH];
};
struct power_on_off_counter
{
	unsigned int counter[POWER_REASON_COUNTER_LENGTH];
};

//warranty information
#define RSV_WARRANTY_ADDR			(0x200 * 0x20)	//1 sector = 512 bytes

//factory information
//RSV backup status
#define RSV_COOKIE_START			0x53565352	/* RSVS */
#define RSV_COOKIE_END				0x45565352	/* RSVE */
#define FS_COOKIE_ID_FORCE_BACKUP_FLAG		0x46		/* 'F' */

enum fs_cookie_id
{
	FS_COOKIE_ID_RECOVERY,
	FS_COOKIE_ID_BACKUP,
	FS_COOKIE_ID_MAX
};

enum fs_back_reco_status
{
	BACKUP_RECOVER_SUCCESS,
	BACKUP_RECOVER_FAIL1,
	BACKUP_RECOVER_FAIL2,
	BACKUP_RECOVER_FAIL3,
	BACKUP_RECOVER_FAIL4,
};

enum fs_process
{
	FS_PROCESS_START,
	FS_PROCESS_END,
};

struct rsv_flag
{
	unsigned int start_magic;	/* STCK */
	unsigned int index;
	unsigned int cookie_id;		/* unique ID for the cookie */
	unsigned int process;
	unsigned int status;
	unsigned int end_magic;		/* in bytes */
};

