/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "sdcard.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

// extern uint8_t SDReadSector(uint32_t addr,uint8_t * buffer);
// extern uint8_t SDReadMultiSector(uint32_t addr, uint8_t sector_num, uint8_t *buffer);
// extern uint8_t SDWriteSector(uint32_t addr, uint8_t *buffer);
// extern uint8_t SDWriteMultiSector(uint32_t addr, uint8_t sector_num, uint8_t *buffer);
extern uint8_t gSdType;

PARTITION VolToPart[]={
	{0,1}
};


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	stat = RES_OK;
	return stat;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	result = 0;
	sdInit();//SDInit();

	if (result == 0)
	{
		stat = RES_OK;
	}
	else
	{
		stat = RES_ERROR;
	}

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT stat;
	int result;
	//BYTE buffd[512]={0};
	
	if(count==1)
	{
		if(gSdType!=0X06)
			result=sdReadDisk(buff,sector*512,1);
		else
			result=sdReadDisk(buff,sector,1);
	}
	else
	{
		if(gSdType!=0X06)
			result = sdReadDisk(buff,sector*512,count);
		else
			result = sdReadDisk(buff,sector,count);
	}
	if (result == 0)
	{
		stat = RES_OK;
	}
	else
	{
		stat = RES_ERROR;
	}
	
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT stat;
	int result;
	if(count==1)
	{
		if(gSdType!=0X06)
			result=sdWriteDisk((uint8_t *)buff,sector*512,1);
			
		else
			result=sdWriteDisk((uint8_t *)buff,sector,1);
	}
	else
	{
		if(gSdType!=0X06)
			result = sdWriteDisk((uint8_t *)buff,sector*512,count);
		else
			result = sdWriteDisk((uint8_t *)buff,sector,count);
	}

	if (result == 0)
	{
		stat = RES_OK;
	}
	else
	{
		stat = RES_ERROR;
	}

	return stat;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT stat;
	//uint8_t csddata[16] = {0};
	//uint32_t csize = 0;
	uint32_t Capacity = 0;
	
	switch (cmd)
	{
	case CTRL_SYNC:
		stat = RES_OK;
		break;
	case GET_SECTOR_COUNT:
		Capacity = sdGetBlockCount();
		*((DWORD *)buff) = Capacity;
		stat = RES_OK;
		break;
	case GET_SECTOR_SIZE:
		*(WORD *)buff = 512; //spi flash的扇区大小是 512 Bytes
		return RES_OK;
	case GET_BLOCK_SIZE:
		*((DWORD *)buff) = 4096;
		stat = RES_OK;
		break;

	default:
		stat = RES_PARERR;
		break;
	}

	return stat;
}

DWORD get_fattime(void)
{
	return 0;
}
