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

#include "luat_base.h"
#include "luat_otp.h"

#ifdef __LUATOS__
#define LUAT_LOG_TAG "otp"
#include "luat_log.h"
#endif

// EC618提供的头文件没有OTP相关的API, 以下调用的均为隐藏API
typedef enum
{
    FLASH_OTP_ERASE = 0,
    FLASH_OTP_WRITE,
    FLASH_OTP_READ,
    FLASH_OTP_LOCK
}FLASH_OTP_OPS;

uint8_t FLASH_otpHandle(FLASH_OTP_OPS opType, uint32_t addr, uint8_t* bufPtr, uint8_t length);

int luat_otp_read(int zone, char* buff, size_t offset, size_t len) {
    uint8_t ret = 0;
    uint32_t addr = ((uint32_t)zone << 12) + offset;
    if (zone >= 1 && zone <= 3) {
        if (offset + len > luat_otp_size(zone)) {
            len = luat_otp_size(zone) - offset;
        }
        //LLOGD("otp read %d %08x %p %d", zone, addr, buff, len);
        ret = FLASH_otpHandle(FLASH_OTP_READ, addr, (uint8_t*)buff, len);
        if (ret == 0) {
            return len;
        }
        return 0;
    }
    return -1;
}

int luat_otp_write(int zone, char* buff, size_t offset, size_t len) {
    uint8_t ret = 0;
    uint32_t addr = ((uint32_t)zone << 12) + offset;
    if (zone >= 1 && zone <= 3) {
        if (offset + len > luat_otp_size(zone)) {
            len = luat_otp_size(zone) - offset;
        }
        //LLOGD("otp write %d %08x %p %d", zone, addr, buff, len);
        ret = FLASH_otpHandle(FLASH_OTP_WRITE, addr, (uint8_t*)buff, len);
        return ret;
    }
    return -1;
}

int luat_otp_erase(int zone, size_t offset, size_t len) {
    (void)offset;
    (void)len;
    uint32_t addr = ((uint32_t)zone << 12);
    if (zone >= 1 && zone <= 3) {
        #ifdef __LUATOS__
        LLOGI("otp erase zone %d %08X", zone, addr);
        #endif
        return FLASH_otpHandle(FLASH_OTP_ERASE, addr, NULL, 0);
    }
    return -1;
}

int luat_otp_lock(int zone) {
    uint32_t addr = ((uint32_t)zone << 12);
    if (zone >= 1 && zone <= 3) {
        #ifdef __LUATOS__
        LLOGW("otp lock zone %d %08X", zone, addr);
        #endif
        return FLASH_otpHandle(FLASH_OTP_LOCK, addr, NULL, 0);
    }
    return -1;
}

size_t luat_otp_size(int zone) {
    if (zone >= 1 && zone <= 3) {
        return 256;
    }
    return 0;
}
