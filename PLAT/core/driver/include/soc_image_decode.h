/*
 * soc_image_decode.h
 *
 *  Created on: 2023年10月24日
 *      Author: Administrator
 */

#ifndef DRIVER_INCLUDE_SOC_IMAGE_DECODE_H_
#define DRIVER_INCLUDE_SOC_IMAGE_DECODE_H_
#include "stdint.h"
int soc_image_decode_init(void *stack, uint32_t stack_bytes, uint32_t priority, uint8_t type);
void soc_image_decode_deinit(void);
int soc_image_decode_once(uint8_t *image_data, uint16_t image_w, uint16_t image_h, uint32_t timeout, CBFuncEx_t cb, void *user_param);
int soc_image_decode_get_result(uint8_t *buf);

#endif /* DRIVER_INCLUDE_SOC_IMAGE_DECODE_H_ */
