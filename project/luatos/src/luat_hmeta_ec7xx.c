#include "luat_base.h"
#include "luat_hmeta.h"
#include "luat_mcu.h"
#include "luat_rtos.h"

extern int soc_get_model_name(char *model, uint8_t is_full);

int luat_hmeta_model_name(char* buff) {
    return soc_get_model_name(buff, 0);
}


int luat_hmeta_hwversion(char* buff2) {
    char buff[30] = {0};
    soc_get_model_name(buff, 1);
    for (size_t i = 0; i < strlen(buff); i++)
    {
        if (buff[i] == '_') {
            memcpy(buff2, buff + i + 1, strlen(buff) - i);
            return 0;
        }
    }
    return -1;
}
