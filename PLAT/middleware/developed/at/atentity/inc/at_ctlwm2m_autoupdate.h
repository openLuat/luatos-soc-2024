/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: at_ctlwm2m_autoupdate.h
*
*  Description: Process ctiot related AT commands header
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _AT_CTLWM2M_AUTOUPDATE_H
#define _AT_CTLWM2M_AUTOUPDATE_H

#define CTLWM2M_AUTO_UPDATE_ENABLE 0

#if CTLWM2M_AUTO_UPDATE_ENABLE
#define CTLWM2M_AUTO_UPDATE_THD (4*3600)
#define CTLWM2M_AUTO_UPDATE_IN_ADVANCE (CTLWM2M_AUTO_UPDATE_THD*1/10)
#define CTLWM2M_LIFETIMEOUT_ID	DEEPSLP_TIMER_ID5
void ctlwm2m_lifeTimeStart(uint32_t timeout_second);
void ctlwm2m_lifeTimeStop(void);
void ctiot_deepSleepTimerCbRegister(void);
#endif

#endif

