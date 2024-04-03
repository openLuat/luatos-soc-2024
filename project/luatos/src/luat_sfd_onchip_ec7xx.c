/*
 * Copyright (c) 2023 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "common_api.h"

#include "luat_base.h"
#include "luat_rtos.h"
#include "luat_mem.h"
#include "luat_debug.h"

#include "stdio.h"
#include "stdlib.h"

#include "flash_rt.h"
#include "mem_map.h"

#ifndef TYPE_EC716S


#include "common_api.h"
#include "luat_base.h"
#include "luat_pm.h"
#include "luat_msgbus.h"

#include "flash_rt.h"
#include "mem_map.h"


#define LUAT_LOG_TAG "sfd"
#include "luat_log.h"

int sfd_onchip_init (void* userdata) {
    return 0;
}

int sfd_onchip_status (void* userdata) {
    return 0;
}

int sfd_onchip_read (void* userdata, char* buf, size_t offset, size_t size) {
    int ret = 0;
    if (size == 0)
        return 0;
    ret = FLASH_readSafe((uint8_t *)buf, (FLASH_FDB_REGION_START + offset), size);
    return ret == 0 ? size : -1;
}

int sfd_onchip_write (void* userdata, const char* buf, size_t offset, size_t size) {
    int ret = 0;
    if (size == 0)
        return 0;
    // 注意, FLASH_writeSafe 的buf不能是flash上的常量数据
    // 写入flash时XIP会关闭, 导致buf值肯定读不到
    // 下面的各种判断, 就是把常量数据拷贝到ram, 然后写入
    uint8_t tmp_small[256]; // 这里改成256的原因是fskv总是按256字节写
    uint8_t *tmp = NULL;
    uint32_t addr = (uint32_t)buf;
    if (size <= 256) {
        // 对于较小的数据, 直接在栈内存里拷贝即可,不必判断
        memcpy(tmp_small, buf, size);
        ret = FLASH_writeSafe((uint8_t *)tmp_small, (FLASH_FDB_REGION_START + offset), size);
    }
    else if (addr >= 0x00400000 && addr <= 0x00500000) {
        // 数据已经处于ram, 可以直接写入
        ret = FLASH_writeSafe((uint8_t *)buf, (FLASH_FDB_REGION_START + offset), size);
    }
    else {
        // 超过256字节的常量数据, 应该是不存在的吧, 下面的逻辑主要是防御代码.
        tmp = malloc(size);
        if (tmp == NULL) {
            DBG("out of memory when malloc flash write buff");
            return -1;
        }
        memcpy(tmp, buf, size);
        ret = FLASH_writeSafe((uint8_t *)tmp, (FLASH_FDB_REGION_START + offset), size);
        free(tmp);
    }
    return ret == 0 ? size : -1;
}
int sfd_onchip_erase (void* userdata, size_t offset, size_t size) {;
    int ret = 0;
    ret = FLASH_eraseSafe((FLASH_FDB_REGION_START + offset), size);
    return ret == 0 ? 0 : -1;
}

int sfd_onchip_ioctl (void* userdata, size_t cmd, void* buff) {
    return 0;
}

#endif
