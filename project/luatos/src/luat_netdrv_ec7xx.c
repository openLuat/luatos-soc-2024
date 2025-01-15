
#include "common_api.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdlib.h>
#include <string.h>//add for memset
#include "bget.h"
#include "luat_base.h"
#include "luat_malloc.h"
#include "luat_netdrv.h"
#include "luat_network_adapter.h"

#define LUAT_LOG_TAG "netdrv"
#include "luat_log.h"

static void gprs_dataout(void* userdata, uint8_t* buff, uint16_t len) {
    extern BOOL PsifRawUlOutput(UINT8, UINT8 *, UINT16);
    // luat_netdrv_print_pkg("上行数据", buff, len);
    BOOL ret = PsifRawUlOutput(1, buff, len);
    LLOGD("gprs 数据上行 %d %p %d ret %d", 1, buff, len, ret);
}

luat_netdrv_t netdrv_gprs = {
    .id = NW_ADAPTER_INDEX_LWIP_GPRS,
    .dataout = gprs_dataout
};

void luat_napt_native_init(void) {
    luat_netdrv_register(NW_ADAPTER_INDEX_LWIP_GPRS, &netdrv_gprs);
}

