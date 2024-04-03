#include "luat_base.h"
#include "luat_hmeta.h"
#include "luat_mcu.h"
#include "luat_rtos.h"

extern int soc_get_model_name(char *model, uint8_t is_full);

int luat_hmeta_model_name(char* buff) {
    int ret = soc_get_model_name(buff, 0);
    if (ret == 0)
        return 0;
    uint64_t ticks = luat_mcu_tick64_ms();
    if (ticks < 250) {
        luat_rtos_task_sleep(250 - ticks);
    }
    return soc_get_model_name(buff, 0);
}


int luat_hmeta_hwversion(char* buff2) {
    char buff[128] = {0};
    int ret = soc_get_model_name(buff, 1);
    if (ret != 0) {
        uint64_t ticks = luat_mcu_tick64_ms();
        if (ticks < 250) {
            luat_rtos_task_sleep(250 - ticks);
        }
        soc_get_model_name(buff, 1);
    }
    for (size_t i = 0; i < strlen(buff); i++)
    {
        if (buff[i] == '_') {
            memcpy(buff2, buff + i + 1, strlen(buff) - i);
            return 0;
        }
    }
    return -1;
}
