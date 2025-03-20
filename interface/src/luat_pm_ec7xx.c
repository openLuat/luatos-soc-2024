#include "luat_base.h"
#include "luat_pm.h"
#include "bsp.h"
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "slpman.h"
#include "reset.h"
#include "driver_gpio.h"
#include "common_api.h"
#include "plat_config.h"
#include "reset.h"
#include "luat_wdt.h"
#include "slpman.h"
#ifdef __LUATOS__
#include "luat_msgbus.h"
#define LUAT_LOG_TAG "pm"
#include "luat_log.h"
#else
#define LLOGW DBG
#define LLOGI DBG
#define LLOGE DBG
#define LLOGD DBG
#endif



static uint8_t lastRequestMode = SLP_IDLE_STATE; // 在APP启动时设置
static uint8_t wakeupSrc = 0xff;
static uint8_t firstSlpstate;
static uint8_t wakeup_deeptimer_id = 0xFF;
static const char *DriverNameText[10]={
		"uart", "soft uart", "usb", "i2c", "spi", "i2s", "adc", "dma", "timer or pwm", "psram"
};
static const char slpStateText[5][5]={{"Actv"},{"Idle"},{"Slp1"},{"Slp2"},{"Hibn"}};
static const char wakeupSrcStr[3][4] = {{"POR"}, {"RTC"}, {"IO"}};
extern void soc_set_usb_sleep(uint8_t onoff);
extern void soc_usb_onoff(uint8_t onoff);
extern int soc_power_mode(uint8_t main, uint8_t sub);
extern void slpManSlpFailedReasonCheck(slpManSlpState_t *DeepestSlpMode, slpManSlpState_t *psphyVoteState, slpManSlpState_t *appVoteState,  slpManSlpState_t *usrdefSlpMode,  uint32_t *drvVoteMap, uint32_t *drvVoteMask);
#ifdef __LUATOS__
extern int luat_dtimer_cb(lua_State *L, void* ptr);
void luat_pm_check_deep_sleep_wakeup_id(void) {
    if ((wakeup_deeptimer_id != 0xff) && luat_msgbus_is_ready())
    {
        rtos_msg_t msg = {0};
        msg.handler = luat_dtimer_cb;
        msg.arg1 = wakeup_deeptimer_id;
        luat_msgbus_put(&msg, 0);
    }
}
#endif
static void appTimerExpFunc(uint8_t id) {
    wakeup_deeptimer_id = id;
    LLOGI("DeepTimer Wakeup by id=%d", id);
#ifdef __LUATOS__
    luat_pm_check_deep_sleep_wakeup_id();
#endif
}

static slpManSlpState_t luat_user_slp_state(void)
{
	return lastRequestMode;
}

int luat_pm_request(int mode) {
    if (mode < 0 || mode > LUAT_PM_SLEEP_MODE_STANDBY) {
        LLOGW("bad mode=%ld", mode);
        return -2;
    }
    if (lastRequestMode < 0 || lastRequestMode > LUAT_PM_SLEEP_MODE_STANDBY)
        lastRequestMode = 0;
    LLOGI("request mode=%s, prev=%s", slpStateText[mode], slpStateText[lastRequestMode]);
    lastRequestMode = mode;
    //soc_set_usb_sleep(1);
    return 0;
}

int luat_pm_release(int mode) {
	soc_set_usb_sleep(0);
	lastRequestMode = LUAT_PM_SLEEP_MODE_IDLE;
    return 0;
}

int luat_pm_dtimer_start(int id, size_t timeout) {
    if (id < 0 || id > DEEPSLP_TIMER_ID6) {
        return -1;
    }
    slpManDeepSlpTimerStart(id, timeout);
    return 0;
}

int luat_pm_dtimer_stop(int id) {
    if (id < 0 || id > DEEPSLP_TIMER_ID6) {
        return -1;
    }
    slpManDeepSlpTimerDel(id);
    return 0;
}

int luat_pm_dtimer_check(int id) {
    if (id < 0 || id > DEEPSLP_TIMER_ID6) {
        return false;
    }
    LLOGD("dtimer check id %d, remain %d ms", id, slpManDeepSlpTimerRemainMs(id));
    if (slpManDeepSlpTimerRemainMs(id) <= 500)
    {
    	slpManDeepSlpTimerDel(id);
    }
    return slpManDeepSlpTimerIsRunning(id);
}

uint32_t luat_pm_dtimer_remain(int id){
    if (id < 0 || id > DEEPSLP_TIMER_ID6) {
        return -1;
    }
	return slpManDeepSlpTimerRemainMs(id);
}

int luat_pm_last_state(int *lastState, int *rtcOrPad) {
    *lastState = firstSlpstate;
    *rtcOrPad = wakeupSrc;
    return 0;
}

int luat_pm_force(int mode) {
    if (mode < 0 || mode > LUAT_PM_SLEEP_MODE_STANDBY) {
        LLOGW("bad mode=%ld", mode);
        return -2;
    }
    LLOGI("force request mode=%ld, prev mode=%ld", mode, lastRequestMode);
	lastRequestMode = mode;
	soc_set_usb_sleep(1);
    return 0;
}

int luat_pm_check(void) {
    slpManSlpState_t DeepestSlpMode;
    slpManSlpState_t psphyVoteState;
    slpManSlpState_t appVoteState;
    slpManSlpState_t usrdefSlpMode;
    uint32_t drvVoteMap;
    uint32_t drvVoteMask;
    slpManSlpState_t final_state = SLP_HIB_STATE;

    slpManSlpFailedReasonCheck(&DeepestSlpMode, &psphyVoteState, &appVoteState, &usrdefSlpMode, &drvVoteMap, &drvVoteMask);
    LLOGI("bsp sleep state %d, %d, %d, %d, %x, %x", DeepestSlpMode, psphyVoteState, appVoteState, usrdefSlpMode, drvVoteMap, drvVoteMask);
    if (DeepestSlpMode < final_state) final_state = DeepestSlpMode;
    if (psphyVoteState < final_state) final_state = psphyVoteState;
    if (appVoteState < final_state) final_state = appVoteState;
    if (usrdefSlpMode < final_state) final_state = usrdefSlpMode;
    LLOGI("bsp sleep mode %d, %s", final_state, slpStateText[final_state]);
    for(uint32_t i = SLP_VOTE_USART; i < SLP_VOTE_MAX_NUM; i++)
    {
    	if ((1 << i) & drvVoteMap)
    	{
    		if ((1 << i) & drvVoteMask)
    		{
    			LLOGI("bsp driver %s running but ignore", DriverNameText[i]);
    		}
    		else
    		{
    			LLOGI("bsp driver %s running, mcu can not sleep", DriverNameText[i]);
    		}
    	}
    }
    return lastRequestMode;
}

int luat_pm_dtimer_list(size_t* c, size_t* dlist) {
    for (uint8_t i = 0; i<= DEEPSLP_TIMER_ID6; i++) {
        if (slpManDeepSlpTimerIsRunning(i)) {
            uint32_t retime = slpManDeepSlpTimerRemainMs(i);
            if (retime != 0xffffffff) {
                *(dlist+i) = retime;
            }
        }
    }
    return 0;
}

int luat_pm_dtimer_wakeup_id(int* id) {
    if (wakeup_deeptimer_id != 0xFF) {
        *id = wakeup_deeptimer_id;
        return 0;
    }
    return -1;
}

//---------------------------------------------------------------
void luat_pm_preinit(void)
{
	for(uint8_t i = 0; i <= DEEPSLP_TIMER_ID6; i++)
	{
	    slpManDeepSlpTimerRegisterExpCb(i, appTimerExpFunc);
	}
}

void luat_pm_init(void) {
#ifdef __LUATOS__
	//LLOGI("pm mode %d", apmuGetDeepestSleepMode());
    if (BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_PWRKEY_MODE) != 0) {
        LLOGD("PowerKey-Debounce is enabled");
        // 开机键防抖处于开启状态, 如需禁用可以调用 pm.power(pm.PWR_MODE, false)
    }
#endif
    if (wakeupSrc != 0xff) return;
    slpManSlpState_t slpstate = slpManGetLastSlpState();
    slpManWakeSrc_e src = slpManGetWakeupSrc();
    wakeupSrc = (uint8_t)src;
    if (src > WAKEUP_FROM_PAD)
    {
    	src = WAKEUP_FROM_PAD;
    }
    if (slpstate == SLP_SLP2_STATE) {
        LLOGI("poweron: Wakup Sleep2 by %s %d", wakeupSrcStr[src], wakeup_deeptimer_id);
        firstSlpstate = LUAT_PM_SLEEP_MODE_DEEP;
    }
    else if (slpstate == SLP_HIB_STATE) {
        LLOGI("poweron: Wakup Hib by %s %d", wakeupSrcStr[src], wakeup_deeptimer_id);
        firstSlpstate = LUAT_PM_SLEEP_MODE_STANDBY;
    }
    else {
        firstSlpstate = LUAT_PM_SLEEP_MODE_NONE;
        LLOGI("poweron: Power/Reset");
    }
    apmuSetDeepestSleepMode(AP_STATE_HIBERNATE);
    slpManRegisterUsrSlpDepthCb(luat_user_slp_state);
#ifdef __LUATOS__
    luat_msgbus_init();
    luat_pm_check_deep_sleep_wakeup_id();
#endif
}

// #endif
int luat_pm_get_poweron_reason(void)
{
    LastResetState_e apRstState,cpRstState;
	ResetStateGet(&apRstState, &cpRstState);
	int id = 0;

	switch(cpRstState)
	{
	case LAST_RESET_HARDFAULT:
	case LAST_RESET_ASSERT:
		return LUAT_PM_POWERON_REASON_EXCEPTION;
		break;
	case LAST_RESET_WDTSW:
	case LAST_RESET_WDTHW:
	case LAST_RESET_LOCKUP:
	case LAST_RESET_AONWDT:
		return LUAT_PM_POWERON_REASON_WDT;
		break;
	default:
		break;
	}

	switch(apRstState)
	{
	case LAST_RESET_CLEAR:
	case LAST_RESET_POR:
		id = LUAT_PM_POWERON_REASON_NORMAL;
		break;
	case LAST_RESET_PAD:
		id = LUAT_PM_POWERON_REASON_HWRESET;
		break;
	case LAST_RESET_SWRESET:
		id = LUAT_PM_POWERON_REASON_SWRESET;
		break;
	case LAST_RESET_HARDFAULT:
	case LAST_RESET_ASSERT:
		id = LUAT_PM_POWERON_REASON_EXCEPTION;
		break;
	case LAST_RESET_WDTSW:
	case LAST_RESET_WDTHW:
	case LAST_RESET_LOCKUP:
	case LAST_RESET_AONWDT:
		id = LUAT_PM_POWERON_REASON_WDT;
		break;
	case LAST_RESET_BATLOW:
	case LAST_RESET_TEMPHI:
		id = LUAT_PM_POWERON_REASON_EXTERNAL;
		break;
	case LAST_RESET_FOTA:
		id = LUAT_PM_POWERON_REASON_FOTA;
		break;
	default:
		id = 200 + cpRstState;
		break;
	}
	return id;
}
///---------------------------------------

int luat_pm_poweroff(void)
{
	luat_wdt_close();
	slpManStartPowerOff();
    return 0;
}

int luat_pm_reset(void)
{
	ResetECSystemReset();
	return 0;
}

int luat_pm_power_ctrl(int id, uint8_t onoff)
{
	switch(id)
	{
	case LUAT_PM_POWER_USB:
		soc_set_usb_sleep(!onoff);
		soc_usb_onoff(onoff);
		break;
	case LUAT_PM_POWER_GPS:
		if (onoff)
		{
			slpManDCXOOutputEn(1);
#ifdef CHIP_EC716
			GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_16, 0), 0, 0, 0);
			GPIO_Config(HAL_GPIO_16, 0, 1);
			GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_13, 0), 0, 0, 0);
			GPIO_Config(HAL_GPIO_13, 0, 1);
#endif
#ifdef CHIP_EC718
#ifdef LUAT_MODEL_AIR8000
			GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_25, 0), 0, 0, 0);
			GPIO_Config(HAL_GPIO_25, 0, 1);
			GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_24, 0), 4, 0, 0);
			GPIO_Config(HAL_GPIO_24, 0, 1);
#else
			GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_23, 0), 0, 0, 0);
			GPIO_Config(HAL_GPIO_23, 0, 1);
			GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_17, 4), 4, 0, 0);
			GPIO_Config(HAL_GPIO_17, 0, 1);
#endif
#endif
		}
		else
		{
#ifdef CHIP_EC716
			GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_13, 0), 0, 0, 0);
			GPIO_Config(HAL_GPIO_13, 0, 0);
			slpManDCXOOutputEn(0);
#endif
#ifdef CHIP_EC718
#ifdef LUAT_MODEL_AIR8000
			GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_25, 0), 0, 0, 0);
			GPIO_Config(HAL_GPIO_25, 0, 0);
#else
			GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_17, 4), 4, 0, 0);
			GPIO_Config(HAL_GPIO_17, 0, 0);
#endif
			slpManDCXOOutputEn(0);
#endif
		}
		break;
	case LUAT_PM_POWER_POWERKEY_MODE:
		if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_PWRKEY_MODE) != onoff)
		{
			LLOGD("powerkey mode %d to %d", BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_PWRKEY_MODE), onoff);
			BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_PWRKEY_MODE, onoff);
			BSP_SavePlatConfigToRawFlash();
		}
		break;
	case LUAT_PM_POWER_WORK_MODE:
		switch(onoff)
		{
		case LUAT_PM_POWER_MODE_NORMAL:
			lastRequestMode = LUAT_PM_SLEEP_MODE_IDLE;
			break;
		case LUAT_PM_POWER_MODE_POWER_SAVER:
			lastRequestMode = LUAT_PM_SLEEP_MODE_STANDBY;
			break;
		default:
			lastRequestMode = LUAT_PM_SLEEP_MODE_LIGHT;
			break;
		}
		return luat_pm_set_power_mode(onoff, 0);
		break;
	case LUAT_PM_POWER_LDO_CTL_PIN:
		GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(HAL_GPIO_16, 4), 4, 0, 0);
		GPIO_Config(HAL_GPIO_16, 0, onoff);
		break;
	default:
		return -1;
	}
	return 0;
}

int luat_pm_iovolt_ctrl(int id, int val) {
	IOVoltageSel_t set;
	if (val > 3400)
	{
		set = IOVOLT_3_40V;
	}
	else if (val >= 2650)
	{
		set = (val - 2650)/50 + IOVOLT_2_65V;
	}
	else if (val > 2000)
	{
		LLOGW("iovolt: out of range %d %d", id, val);
		return -1;
	}
	else if (val >= 1650)
	{
		set = (val - 1650)/50;
	}
	else
	{
		set = IOVOLT_1_65V;
	}
	slpManNormalIOVoltSet(set);
	slpManAONIOVoltSet(set);
	return 0;

}

int luat_pm_wakeup_pin(int pin, int val){
    LLOGW("not support yet");
    return -1;
}

int luat_pm_set_power_mode(uint8_t mode, uint8_t sub_mode)
{
	return soc_power_mode(mode, sub_mode);
}

