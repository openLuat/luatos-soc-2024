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

/*
 * mem操作
 * 
 */

#include <stdlib.h>
#include "luat_base.h"
#include "common_api.h"
#include "mem_map.h"
#include "luat_mem.h"
//extern uint32_t start_up_buffer;
//extern uint32_t end_ap_data;
//uint32_t sys_heap_start = &(start_up_buffer);
//uint32_t sys_heap_end = &(end_ap_data);

#include "FreeRTOS.h"
extern void GetSRAMHeapInfo(uint32_t *total, uint32_t *alloc, uint32_t *peak);
extern void GetPSRAMHeapInfo(uint32_t *total, uint32_t *alloc, uint32_t *peak);

void* luat_heap_malloc(size_t len) {
    return malloc(len);
}

void luat_heap_free(void* ptr) {
//	if ((uint32_t)ptr > sys_heap_start && (uint32_t)ptr <= sys_heap_end) {
	//先做简单判断，如果有问题，再做严格判断
	if ((uint32_t)ptr > MSMB_START_ADDR && (uint32_t)ptr <= up_buf_start) {
		free(ptr);
		return ;
	}
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	if ((uint32_t)ptr > PSRAM_START_ADDR && (uint32_t)ptr <= PSRAM_END_ADDR) {
		vPortFree_Psram(ptr);
		return ;
	}
#endif
	DBG("invaild ptr %p", ptr);
}

void* luat_heap_realloc(void* ptr, size_t len) {
    return realloc(ptr, len);
}

void* luat_heap_calloc(size_t count, size_t _size) {
    return calloc(count, _size);
}

void luat_meminfo_sys(size_t *total, size_t *used, size_t *max_used) {
	GetSRAMHeapInfo(total, used, max_used);
}

void* luat_heap_opt_malloc(LUAT_HEAP_TYPE_E type,size_t len){
	if (type == LUAT_HEAP_AUTO){
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
		void* _ptr = pvPortMalloc_Psram(len);
		if (_ptr) return _ptr;
		else
#endif
			return malloc(len);
	}
	else if(type == LUAT_HEAP_SRAM) return malloc(len);
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	else if(type == LUAT_HEAP_PSRAM) return pvPortMalloc_Psram(len);
#endif
	else return NULL;
}

void luat_heap_opt_free(LUAT_HEAP_TYPE_E type,void* ptr){
	luat_heap_free(ptr);
}

void* luat_heap_opt_realloc(LUAT_HEAP_TYPE_E type,void* ptr, size_t len){
	if (type == LUAT_HEAP_AUTO){
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
		void* _ptr = pvPortRealloc_Psram(ptr,len);
		if (_ptr) return _ptr;
		else
#endif
			return realloc(ptr, len);
	}
	else if(type == LUAT_HEAP_SRAM) return realloc(ptr, len);
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	else if(type == LUAT_HEAP_PSRAM) return pvPortRealloc_Psram(ptr,len);
#endif
	else return NULL;
}

void* luat_heap_opt_zalloc(LUAT_HEAP_TYPE_E type,size_t size){
	if (type == LUAT_HEAP_AUTO){
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
		void* _ptr = pvPortZeroMalloc_Psram(1 * size);
		if (_ptr) return _ptr;
		else
#endif
			return calloc(1, size);
	}
	else if(type == LUAT_HEAP_SRAM) return calloc(1, size);
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	else if(type == LUAT_HEAP_PSRAM) return pvPortZeroMalloc_Psram(1 * size);
#endif
	else return NULL;
}

void luat_meminfo_opt_sys(LUAT_HEAP_TYPE_E type,size_t* total, size_t* used, size_t* max_used){
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	if(type == LUAT_HEAP_PSRAM) GetPSRAMHeapInfo(total, used, max_used);
	else 
#endif
		GetSRAMHeapInfo(total, used, max_used);
}

