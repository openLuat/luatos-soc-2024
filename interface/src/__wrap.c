#include "common_api.h"
#include "time.h"
#include "osasys.h"
#include "mw_aon_info.h"
extern MidWareAonInfo      *pMwAonInfo;
//static void RTC_GetDateTime(Date_UserDataStruct *pDate, Time_UserDataStruct *pTime)
//{
//	PV_Union uPV;
//	utc_timer_value_t *timeUtc = OsaSystemTimeReadRamUtc();
//	uPV.u32 = timeUtc->UTCtimer1;
//	pDate->Year = uPV.u16[1];
//	pDate->Mon = uPV.u8[1];
//	pDate->Day = uPV.u8[0];
//	uPV.u32 = timeUtc->UTCtimer2;
//	pTime->Hour = uPV.u8[3];
//	pTime->Min = uPV.u8[2];
//	pTime->Sec = uPV.u8[1];
//	//DBG("%d,%d,%d,%d,%d,%d",pDate->Year,pDate->Mon,pDate->Day,pTime->Hour,pTime->Min,pTime->Sec);
//	//Tamp2UTC(timeUtc->UTCsecs, pDate, pTime, 0);
//}

static struct tm prvTM;
extern const uint32_t DayTable[2][12];
__attribute__((used)) struct tm * __wrap_localtime (const time_t *_timer)
{
	Time_UserDataStruct Time;
	Date_UserDataStruct Date;
	uint64_t Sec = 1732535217;
	int64_t tz = 32;

	if (pMwAonInfo)
	{
		uint32_t tick = slpManGet6P25HZGlobalCnt();
		uint32_t cr = OS_EnterCritical();
		if (pMwAonInfo->crc16 == CRC16Cal(&pMwAonInfo->utc_tamp, 9, CRC16_CCITT_SEED, CRC16_CCITT_GEN, 0))
		{
			Sec = pMwAonInfo->utc_tamp;
			uint64_t diff = (tick - pMwAonInfo->rtc_tamp);
			Sec += diff * 4 / 25;
			tz = pMwAonInfo->tz;
		}
		else
		{
			DBG("rtc record error!");
		}
		OS_ExitCritical(cr);
	}

	if (_timer)
	{
		Sec = *_timer;
	}

	Tamp2UTC(Sec + tz * 900, &Date, &Time, 0);
	prvTM.tm_year = Date.Year - 1900;
	prvTM.tm_mon = Date.Mon - 1;
	prvTM.tm_mday = Date.Day;
	prvTM.tm_hour = Time.Hour;
	prvTM.tm_min = Time.Min;
	prvTM.tm_sec = Time.Sec;
	prvTM.tm_wday = Time.Week;
	prvTM.tm_yday = Date.Day - 1;
	prvTM.tm_yday += DayTable[IsLeapYear(Date.Year)][Date.Mon - 1];
	return &prvTM;
}

__attribute__((used)) struct tm * __wrap_gmtime (const time_t *_timer)
{
	Time_UserDataStruct Time;
	Date_UserDataStruct Date;
	int64_t Sec;
	if (_timer)
	{
		Tamp2UTC(*_timer, &Date, &Time, 0);
	}
	else
	{

		uint64_t Sec = 1732535217;
		if (pMwAonInfo)
		{
			uint32_t tick = slpManGet6P25HZGlobalCnt();
			uint32_t cr = OS_EnterCritical();
			if (pMwAonInfo->crc16 == CRC16Cal(&pMwAonInfo->utc_tamp, 9, CRC16_CCITT_SEED, CRC16_CCITT_GEN, 0))
			{
				Sec = pMwAonInfo->utc_tamp;
				uint64_t diff = (tick - pMwAonInfo->rtc_tamp);
				Sec += diff * 4 / 25;
			}
			else
			{
				DBG("rtc record error!");
			}
			OS_ExitCritical(cr);
		}
		Tamp2UTC(Sec, &Date, &Time, 0);
	}
	prvTM.tm_year = Date.Year - 1900;
	prvTM.tm_mon = Date.Mon - 1;
	prvTM.tm_mday = Date.Day;
	prvTM.tm_hour = Time.Hour;
	prvTM.tm_min = Time.Min;
	prvTM.tm_sec = Time.Sec;
	prvTM.tm_wday = Time.Week;
	prvTM.tm_yday = Date.Day - 1;
	prvTM.tm_yday += DayTable[IsLeapYear(Date.Year)][Date.Mon - 1];
	return &prvTM;
}

__attribute__((used)) clock_t __wrap_clock (void)
{
	return GetSysTickMS()/1000;
}

__attribute__((used)) time_t __wrap_time (time_t *_Time)
{
	time_t Sec = 1732535217;
	if (pMwAonInfo)
	{
		uint32_t tick = slpManGet6P25HZGlobalCnt();
		uint32_t cr = OS_EnterCritical();
		if (pMwAonInfo->crc16 == CRC16Cal(&pMwAonInfo->utc_tamp, 9, CRC16_CCITT_SEED, CRC16_CCITT_GEN, 0))
		{
			Sec = pMwAonInfo->utc_tamp;
			time_t diff = (tick - pMwAonInfo->rtc_tamp);
			Sec += diff * 4 / 25;
		}
		else
		{
			DBG("rtc record error!");
		}
		OS_ExitCritical(cr);
	}
	  if (_Time != NULL) {
	    *_Time = Sec;
	  }
	  return Sec;
}

#include "bsp.h"
uint8_t* __wrap_getBuildInfo(void)
{
    return (uint8_t *)(
		STRING_EOL"-- Board: " CHIP_TYPE " -- "STRING_EOL \
                         VERSION_INFO STRING_EOL
	);
}
