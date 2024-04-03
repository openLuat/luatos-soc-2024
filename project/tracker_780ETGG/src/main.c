#include "common_api.h"
#include "network_service.h"
#include "socket_service.h"
#include "usb_service.h"
#include "luat_sms_app.h"
#include "gpsmsg.h"


extern void luat_service_init(void);
extern void fota_task_init(void);

INIT_TASK_EXPORT(network_service_init, "0");
INIT_TASK_EXPORT(socket_service_init, "1");
//INIT_TASK_EXPORT(usb_service_init, "2");
INIT_TASK_EXPORT(gps_service_init, "2");
INIT_TASK_EXPORT(agps_service_init, "3");
//INIT_TASK_EXPORT(fota_task_init, "3");
INIT_TASK_EXPORT(luat_service_init, "4");
INIT_TASK_EXPORT(luat_sms_task_init, "4");

