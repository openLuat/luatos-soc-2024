#include "wdt.h"
#include "clock.h"
int luat_wdt_init(size_t timeout)
{
    if(timeout < 1 || timeout > 60)
    {
        return -1;
    }
    WDT_kick();
    WDT_stop();
    WDT_deInit();
    GPR_setClockSrc(FCLK_WDG, FCLK_WDG_SEL_40K);
    GPR_setClockDiv(FCLK_WDG, timeout);

    WdtConfig_t wdtConfig;
    wdtConfig.mode = WDT_INTERRUPT_RESET_MODE;
    wdtConfig.timeoutValue = 40000U;
    WDT_init(&wdtConfig);
    WDT_start();
    return 0;
}

int luat_wdt_set_timeout(size_t timeout)
{
    if(timeout < 1 || timeout > 60)
    {
        return -1;
    }
    luat_wdt_init(timeout);
    return 0;
}

int luat_wdt_feed(void)
{
    WDT_kick();
    slpManAonWdtFeed();
    return 0;
}

int luat_wdt_close(void)
{
    WDT_stop();
    WDT_deInit();
    return 0;
}
