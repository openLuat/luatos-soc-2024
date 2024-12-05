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

int luat_hmeta_chip(char* buff) {
    #ifdef TYPE_EC718UM
        strcpy(buff, "EC718UM");
    #elif defined(TYPE_EC718PM)
        strcpy(buff, "EC718PM");
    #elif defined(TYPE_EC718HM)
        strcpy(buff, "EC718HM");
    #elif defined(TYPE_EC718PM)
        strcpy(buff, "EC718PM");
    #elif defined(TYPE_EC718U)
        strcpy(buff, "EC718U");
    #elif defined(TYPE_EC718PV)
        strcpy(buff, "EC718PV");
    #elif defined(TYPE_EC718P)
        strcpy(buff, "EC718P");
    #elif defined(TYPE_EC718E)
        strcpy(buff, "EC718E");
    #elif defined(TYPE_EC718S)
        strcpy(buff, "EC718S");
    #elif defined(TYPE_EC716E)
        strcpy(buff, "EC716E");
    #elif defined(TYPE_EC716S)
        strcpy(buff, "EC716S");
    #endif
    return 0;
}
