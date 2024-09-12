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
#include "luat_debug.h"
#include "soc_service.h"
//extern uint32_t start_up_buffer;
//extern uint32_t end_ap_data;
//uint32_t sys_heap_start = &(start_up_buffer);
//uint32_t sys_heap_end = &(end_ap_data);

#include "FreeRTOS.h"
extern void GetSRAMHeapInfo(uint32_t *total, uint32_t *alloc, uint32_t *peak);
extern void GetPSRAMHeapInfo(uint32_t *total, uint32_t *alloc, uint32_t *peak);

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
static llist_head prv_psram_record_list_head;
typedef struct
{
	llist_head node;
	void *address;
}psram_record_t;


static int find_record(void *node, void *ptr)
{
	psram_record_t *record = (psram_record_t *)node;
	return (ptr == record->address);
}


static void *psram_malloc(size_t len)
{
	void* _ptr = pvPortMalloc_Psram(len);
	if (_ptr)
	{
		luat_rtos_task_suspend_all();
		if (!prv_psram_record_list_head.next || !prv_psram_record_list_head.prev)
		{
			INIT_LLIST_HEAD(&prv_psram_record_list_head);
		}
		psram_record_t *record = pvPortAssertMalloc(sizeof(psram_record_t));
		record->address = _ptr;
		if (llist_empty(&prv_psram_record_list_head))
		{
			soc_sys_force_wakeup_on_off(SOC_SYS_CTRL_PSRAM, 1);
		}
		llist_add_tail(&record->node, &prv_psram_record_list_head);
		luat_rtos_task_resume_all();
	}
	return _ptr;
}

static void *psram_realloc(void* ptr, size_t len)
{
	void* _ptr = pvPortRealloc_Psram(ptr, len);
	if (_ptr)
	{
		luat_rtos_task_suspend_all();
		if (!prv_psram_record_list_head.next || !prv_psram_record_list_head.prev)
		{
			INIT_LLIST_HEAD(&prv_psram_record_list_head);
		}
		if (ptr)
		{
			psram_record_t *record =llist_traversal(&prv_psram_record_list_head, find_record, ptr);
			LUAT_DEBUG_ASSERT(record, "no psram record");
			record->address = _ptr;
		}
		else
		{
			psram_record_t *record = pvPortAssertMalloc(sizeof(psram_record_t));
			record->address = _ptr;
			if (llist_empty(&prv_psram_record_list_head))
			{
				soc_sys_force_wakeup_on_off(SOC_SYS_CTRL_PSRAM, 1);
			}
			llist_add_tail(&record->node, &prv_psram_record_list_head);
		}
		luat_rtos_task_resume_all();

	}
	return _ptr;
}

static void psram_free(void *ptr)
{
	vPortFree_Psram(ptr);
	luat_rtos_task_suspend_all();
	if (!prv_psram_record_list_head.next || !prv_psram_record_list_head.prev)
	{
		LUAT_DEBUG_ASSERT(0, "psram record not init");
	}
	psram_record_t *record =llist_traversal(&prv_psram_record_list_head, find_record, ptr);
	LUAT_DEBUG_ASSERT(record, "no psram record");
	llist_del(&record->node);
	free(record);
	if (llist_empty(&prv_psram_record_list_head))
	{
		soc_sys_force_wakeup_on_off(SOC_SYS_CTRL_PSRAM, 0);
	}
	luat_rtos_task_resume_all();
}
#endif

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
		psram_free(ptr);
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
		void* _ptr = psram_malloc(len);
		if (_ptr) return _ptr;
		else
#endif
			return malloc(len);
	}
	else if(type == LUAT_HEAP_SRAM) return malloc(len);
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	else if(type == LUAT_HEAP_PSRAM) return psram_malloc(len);
#endif
	else return NULL;
}

void luat_heap_opt_free(LUAT_HEAP_TYPE_E type,void* ptr){
	luat_heap_free(ptr);
}

void* luat_heap_opt_realloc(LUAT_HEAP_TYPE_E type,void* ptr, size_t len){
	if (type == LUAT_HEAP_AUTO){
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
		void* _ptr = psram_realloc(ptr,len);
		if (_ptr) return _ptr;
		else
#endif
			return realloc(ptr, len);
	}
	else if(type == LUAT_HEAP_SRAM) return realloc(ptr, len);
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	else if(type == LUAT_HEAP_PSRAM) return psram_realloc(ptr,len);
#endif
	else return NULL;
}

void* luat_heap_opt_zalloc(LUAT_HEAP_TYPE_E type,size_t size){
	if (type == LUAT_HEAP_AUTO){
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
		void* _ptr = psram_malloc(size);
		if (_ptr)
		{
			memset(_ptr, 0, size);
			return _ptr;
		}
		else
#endif
			return calloc(1, size);
	}
	else if(type == LUAT_HEAP_SRAM) return calloc(1, size);
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	else if(type == LUAT_HEAP_PSRAM)
	{
		void* _ptr = psram_malloc(size);
		if (_ptr)
		{
			memset(_ptr, 0, size);

		}
		return _ptr;
	}
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

