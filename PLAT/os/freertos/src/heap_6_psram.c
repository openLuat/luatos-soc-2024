
#include "FreeRTOS.h"
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
#if( configSUPPORT_DYNAMIC_ALLOC_HEAP == 6 )
#include <string.h>//add for memset
#include <stdbool.h>
#include "sctdef.h"
#include "task.h"
#include "mm_debug.h"//add for memory leak debug
#include "exception_process.h"
#include "cmsis_compiler.h"
#include "tlsf.h"
#include "mem_map.h"
#include "sctdef.h"

#ifdef TYPE_EC718M
#define CUST_HEAP_6_RAMCODE CUST_FPSRAM_P2_RAMCODE
#define CUST_HEAP_6_DATA    CUST_FPSRAM_P2_DATA
#define CUST_HEAP_6_ZI      CUST_FPSRAM_P2_BSS
#else
#define CUST_HEAP_6_RAMCODE PLAT_PSRAM_HEAP6_RAMCODE
#define CUST_HEAP_6_DATA
#define CUST_HEAP_6_ZI      PLAT_FPSRAM_HEAP6_ZI
#endif

#if( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
    #error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif

extern uint8_t gucPoolGroupSel;

#ifdef CORE_IS_AP
    extern UINT32 heap_endAddr_psram;
    extern UINT32 end_ap_data_psram;
#endif

/* Allocate the memory for the heap. */
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
    /* The application writer has already defined the array used for the RTOS
     * heap - probably so it can be placed in a special segment or address.
     */
    CUST_HEAP_6_ZI uint8_t ucHeap_psram[ configTOTAL_HEAP_SIZE ];
#else
    //static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
    #ifdef CORE_IS_AP
    CUST_HEAP_6_DATA uint8_t * ucHeap_psram=(uint8_t *)&( end_ap_data_psram );
    #if MM_TRACE_ON == 2
    #define TLSF_AP_HEAP_PSRAM_MAX  (128*1024)        // worse case of heap size
    #else
    #define TLSF_AP_HEAP_PSRAM_MAX  tlsf_block_size_max()
    #endif
    #else
    static CUST_HEAP_6_ZI uint8_t ucHeap_psram[ configTOTAL_HEAP_SIZE ];//cp still use fix length array
    #endif

    //dynamic heap size, caculate per compilation
    CUST_HEAP_6_ZI UINT32 gTotalHeapSize_psram=0;

#endif /* configAPPLICATION_ALLOCATED_HEAP */


/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit_Psram( void );

#ifdef TYPE_EC718M
static tlsf_t    pxTlsf_psram = NULL;
#else
PLAT_FPSRAM_ZI static tlsf_t    pxTlsf_psram = NULL;
#endif
CUST_HEAP_6_RAMCODE void *pvPortZeroMalloc_Psram( size_t xWantedSize)
{
    void *ptr = pvPortMallocEC_Psram(xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
    return ptr ? memset(ptr, 0, xWantedSize), ptr : ptr;
}

CUST_HEAP_6_RAMCODE void *pvPortAssertMalloc_Psram( size_t xWantedSize)
{
    void *ptr = pvPortMallocEC_Psram(xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
    configASSERT(ptr != 0);
    return ptr;
}


CUST_HEAP_6_RAMCODE void *pvPortZeroAssertMalloc_Psram( size_t xWantedSize)
{
    void *ptr = pvPortMallocEC_Psram(xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
    configASSERT(ptr != 0);
    memset(ptr, 0, xWantedSize);
    return ptr;
}

CUST_HEAP_6_RAMCODE void *pvPortMallocEC_Psram( size_t xWantedSize, unsigned int funcPtr )
{
    void *pvReturn = NULL;

    configASSERT(__get_IPSR() == 0 && "no invokation by IPSR!");

#ifdef MEM_BLK_SIZE_32BIT
    configASSERT(xWantedSize > 0 && "zero alloc is prohibited!");
#else
    configASSERT(xWantedSize > 0 && xWantedSize < 0x10000 && "0 or 64K(+) alloc is prohibited!");
#endif

    vTaskSuspendAll();
    {
        if(NULL == pxTlsf_psram)
        {
            prvHeapInit_Psram();

        #ifdef MM_DEBUG_EN
            mm_trace_init();
        #endif
        }
    #ifdef MM_DEBUG_EN
        if(funcPtr == 0)
        {
            funcPtr = (unsigned int)__GET_RETURN_ADDRESS();
        }
    #endif
        pvReturn = tlsf_malloc(pxTlsf_psram, xWantedSize, (size_t)funcPtr);
    #ifdef MM_DEBUG_EN
        if( pvReturn != NULL )
        {
            mm_malloc_trace(pvReturn, xWantedSize, funcPtr);
        }
    #endif
    }
    xTaskResumeAll();

    configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );

    return pvReturn;
}

CUST_HEAP_6_RAMCODE void *pvPortReallocEC_Psram( void *pv, size_t xWantedSize,  unsigned int funcPtr )
{
    void *pvReturn = NULL;

    configASSERT(__get_IPSR() == 0 && "no invokation by IPSR!");

#ifdef MEM_BLK_SIZE_32BIT
    configASSERT(xWantedSize > 0 && "zero alloc is prohibited!");
#else
    configASSERT(xWantedSize > 0 && xWantedSize < 0x10000 && "0 or 64K(+) alloc is prohibited!");
#endif

    vTaskSuspendAll();
    {
        /* do the initialization job when invoked for the first time! */
        if(NULL == pxTlsf_psram)
        {
            prvHeapInit_Psram();
        }
    #ifdef MM_DEBUG_EN
        if(funcPtr == 0)
        {
            funcPtr = (unsigned int)__GET_RETURN_ADDRESS();
        }
    #endif
        pvReturn = tlsf_realloc(pxTlsf_psram, pv, xWantedSize, funcPtr);
    #ifdef MM_DEBUG_EN
        if( pvReturn != NULL )
        {
            mm_malloc_trace(pvReturn, xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
        }
    #endif
    }
    xTaskResumeAll();

    configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );

    return pvReturn;
}

CUST_HEAP_6_RAMCODE void  vPortFree_Psram( void *pv )
{
    configASSERT(__get_IPSR() == 0 && "no invokation by IPSR!");

    vTaskSuspendAll();
    {
        if(pxTlsf_psram && pv)
        {
            tlsf_free(pxTlsf_psram, pv);

        #ifdef MM_DEBUG_EN
            mm_free_trace(pv);
        #endif
        }
    }
    xTaskResumeAll();
}

CUST_HEAP_6_RAMCODE size_t xPortGetTotalHeapSize_Psram( void )
{
    return gTotalHeapSize_psram;
}

CUST_HEAP_6_RAMCODE size_t xPortGetFreeHeapSize_Psram( void )
{
    if(!pxTlsf_psram) return 0;

    uint32_t mask = SaveAndSetIRQMask();
    gucPoolGroupSel = 1;
    size_t size = tlsf_mem_size_free(pxTlsf_psram);
    gucPoolGroupSel = 0;
    RestoreIRQMask(mask);

    return size;
}

CUST_HEAP_6_RAMCODE uint8_t xPortGetFreeHeapPct_Psram( void )
{
    if(!pxTlsf_psram) return 0;

    return (uint8_t)((xPortGetFreeHeapSize_Psram() * 100) / xPortGetTotalHeapSize_Psram());
}

CUST_HEAP_6_RAMCODE size_t xPortGetMaximumFreeBlockSize_Psram( void )
{
    if(!pxTlsf_psram) return 0;

    uint32_t mask = SaveAndSetIRQMask();

    size_t size = tlsf_mem_max_block_size(pxTlsf_psram);

    RestoreIRQMask(mask);

    return size;
}

#define portHEAP_TOTAL_FREE_ALERT_PCT_PSRAM   30
#define portHEAP_FREE_BLOCK_ALERT_SIZE_PSRAM  8192

CUST_HEAP_6_RAMCODE uint8_t xPortIsFreeHeapOnAlert_Psram( void )
{
    return ((xPortGetFreeHeapPct_Psram() <= portHEAP_TOTAL_FREE_ALERT_PCT_PSRAM) || \
            (xPortGetMaximumFreeBlockSize_Psram() <= portHEAP_FREE_BLOCK_ALERT_SIZE_PSRAM)) ? 1 : 0;
}

CUST_HEAP_6_RAMCODE size_t xPortGetMinimumEverFreeHeapSize_Psram( void )
{
    if(!pxTlsf_psram) return 0;

    uint32_t mask = SaveAndSetIRQMask();
    gucPoolGroupSel = 1;
    size_t size = tlsf_mem_size_ever_min(pxTlsf_psram);
    gucPoolGroupSel = 0;
    RestoreIRQMask(mask);

    return size;
}

CUST_HEAP_6_RAMCODE void  vPortShowMemRecord_Psram( void )
{
    if(!pxTlsf_psram)
    {
        return;
    }
    gucPoolGroupSel = 1;
    tlsf_show_mem_record(pxTlsf_psram);
    gucPoolGroupSel = 0;
}

CUST_HEAP_6_RAMCODE void  vPortClearMemRecord_Psram( void )
{
    if(!pxTlsf_psram)
    {
        return;
    }
    tlsf_clear_mem_record(pxTlsf_psram);
}

CUST_HEAP_6_RAMCODE void  vPortShowPhysMemBlock_Psram(void *callback, int type, int *mem_range)
{
    int block_type;
    if(pxTlsf_psram)
    {
        if(type <= 2)
        {
            tlsf_set_print_callback(callback);
            block_type = type;
            tlsf_check_pool(tlsf_get_pool(pxTlsf_psram), block_type);
            tlsf_show_block_detail(pxTlsf_psram);
            tlsf_show_hist_min_max_free_block(pxTlsf_psram, NULL);
            tlsf_show_cur_max_block(pxTlsf_psram, NULL);
        }
    }
}

CUST_HEAP_6_RAMCODE bool vPortGetHeapInfo_Psram(uint8_t type, int *mem_range)
{
    if(pxTlsf_psram != NULL)
    {
        tlsf_set_print_callback(NULL);
        if(type == 0)
        {
            tlsf_show_hist_min_max_free_block(pxTlsf_psram, mem_range);
        }
        else if(type == 1)
        {
            tlsf_show_cur_max_block(pxTlsf_psram, mem_range);
        }
        return true;
    }
    return false;
}

CUST_HEAP_6_RAMCODE static void prvHeapInit_Psram( void )
{
#ifdef CORE_IS_AP
    gTotalHeapSize_psram = (UINT32)&(heap_endAddr_psram) - (UINT32)&(end_ap_data_psram);
#ifdef __USER_CODE__
#else
    gTotalHeapSize_psram = (gTotalHeapSize_psram>TLSF_AP_HEAP_PSRAM_MAX) ? TLSF_AP_HEAP_PSRAM_MAX : gTotalHeapSize_psram;
#endif
#else
    gTotalHeapSize_psram = configTOTAL_HEAP_SIZE;
#endif
    gucPoolGroupSel = 1;
    pxTlsf_psram = tlsf_create_with_pool(ucHeap_psram, gTotalHeapSize_psram);
    gucPoolGroupSel = 0;
}

#ifdef __USER_CODE__
extern void tlsf_mem_get_record(tlsf_t tlsf, uint32_t *alloc, uint32_t *peak);
FREERTOS_HEAP6_TEXT_SECTION void GetPSRAMHeapInfo(uint32_t *total, uint32_t *alloc, uint32_t *peak)
{
	vTaskSuspendAll();
    if(NULL == pxTlsf_psram)
    {
        prvHeapInit_Psram();
    }
	*total = gTotalHeapSize_psram;
	tlsf_mem_get_record(pxTlsf_psram, alloc, peak);
	xTaskResumeAll();

}
#endif

#endif
#endif

