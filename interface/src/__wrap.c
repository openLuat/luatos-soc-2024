#include "common_api.h"
#include "time.h"
#include "osasys.h"

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
	int64_t Sec;
	utc_timer_value_t *timeUtc = OsaSystemTimeReadRamUtc();
	if (_timer)
	{
		Sec = *_timer;
		Tamp2UTC(Sec + timeUtc->timeZone * 900, &Date, &Time, 0);
	}
	else
	{
//		RTC_GetDateTime(&Date, &Time);
//		int64_t tamp = UTC2Tamp(&Date, &Time);
		Tamp2UTC(timeUtc->UTCsecs + timeUtc->timeZone * 900, &Date, &Time, 0);
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

__attribute__((used)) struct tm * __wrap_gmtime (const time_t *_timer)
{
	Time_UserDataStruct Time;
	Date_UserDataStruct Date;
	int64_t Sec;
	if (_timer)
	{
		Sec = *_timer;
		Tamp2UTC(Sec, &Date, &Time, 0);
	}
	else
	{
		utc_timer_value_t *timeUtc = OsaSystemTimeReadRamUtc();
//		RTC_GetDateTime(&Date, &Time);
//		int64_t tamp = UTC2Tamp(&Date, &Time);
		Tamp2UTC(timeUtc->UTCsecs, &Date, &Time, 0);
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
  utc_timer_value_t *timeUtc = OsaSystemTimeReadRamUtc();
  if (_Time != NULL) {
    *_Time = timeUtc->UTCsecs;
  }
  return timeUtc->UTCsecs;
}

#include "bsp.h"
uint8_t* __wrap_getBuildInfo(void)
{
    return (uint8_t *)(
		STRING_EOL"-- Board: " CHIP_TYPE " -- "STRING_EOL \
                         VERSION_INFO STRING_EOL
	);
}
