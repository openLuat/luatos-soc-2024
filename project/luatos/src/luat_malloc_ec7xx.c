
#include "common_api.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdlib.h>
#include <string.h>//add for memset
#include "bget.h"
#include "luat_base.h"
#include "luat_malloc.h"
#include "luat_mem.h"

#define LUAT_LOG_TAG "vmheap"
#include "luat_log.h"


#include "mm_debug.h"//add for memory leak debug
#include "exception_process.h"
#include "cmsis_compiler.h"
#include "tlsf.h"
#include "mem_map.h"

#ifndef LUAT_USE_PSRAM_AS_LUAHEAP
#define LUAT_USE_PSRAM_AS_LUAHEAP 0
#endif

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1) && (LUAT_USE_PSRAM_AS_LUAHEAP==1)
#undef LUAT_HEAP_SIZE
#define LUAT_HEAP_SIZE (1280*1024) // 1280k 1.25M
static uint8_t* vmheap;
#else
#ifndef LUAT_HEAP_SIZE
#define LUAT_HEAP_SIZE (200*1024)
#endif
static uint8_t vmheap[LUAT_HEAP_SIZE] __attribute__((aligned(8)));
#endif


//------------------------------------------------
// ---------- 管理 LuaVM所使用的内存----------------


void luat_heap_init(void) {
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1) && (LUAT_USE_PSRAM_AS_LUAHEAP==1)
    vmheap = luat_heap_opt_malloc(LUAT_HEAP_PSRAM, LUAT_HEAP_SIZE);
#endif
    bpool(vmheap, LUAT_HEAP_SIZE);
}

void* luat_heap_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    if (ptr == NULL && nsize == 0)
        return NULL;
#if LUAT_USE_MEMORY_OPTIMIZATION_CODE_MMAP
    if (ptr != NULL) {
        uint32_t addr = (uint32_t) ptr;
        if (addr <= (uint32_t)vmheap || addr >= (uint32_t)(vmheap + LUAT_HEAP_SIZE)) {
            //LLOGD("skip ROM free %p", ptr);
            return NULL;
        }
    }
#endif

    if (nsize)
    {
    	void* ptmp = bgetr(ptr, nsize);
    	if(ptmp == NULL && osize >= nsize)
    	{
    		return ptr;
    	}
        return ptmp;
    }
    brel(ptr);
    return NULL;
}

void luat_meminfo_luavm(size_t *total, size_t *used, size_t *max_used) {
	long curalloc, totfree, maxfree;
	unsigned long nget, nrel;
	bstats(&curalloc, &totfree, &maxfree, &nget, &nrel);
	*used = curalloc;
	*max_used = bstatsmaxget();
    *total = curalloc + totfree;
}


//-----------------------------------------------------------------------------

