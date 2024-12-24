
#include "FreeRTOS.h"
#if (defined __USER_CODE__) && (defined TYPE_EC718M)
#else
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
    CUST_HEAP_6_ZI uint8_t ucHeap_Cust[ configTOTAL_HEAP_SIZE ];
#else
    //static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
    #ifdef CORE_IS_AP
    CUST_HEAP_6_DATA uint8_t * ucHeap_Cust=(uint8_t *)&( end_ap_data_psram );
    #if MM_TRACE_ON == 2
    #define TLSF_AP_HEAP_MAX_CUST  (128*1024)        // worse case of heap size
    #else
    #define TLSF_AP_HEAP_MAX_CUST  tlsf_block_size_max()
    #endif
    #else
    static CUST_HEAP_6_ZI uint8_t ucHeap_Cust[ configTOTAL_HEAP_SIZE ];//cp still use fix length array
    #endif

    //dynamic heap size, caculate per compilation
    CUST_HEAP_6_ZI UINT32 gTotalHeapSize_Cust=0;

#endif /* configAPPLICATION_ALLOCATED_HEAP */


/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInitCust( void );

#ifdef TYPE_EC718M
static tlsf_t    pxTlsf_Cust = NULL;
#else
PLAT_FPSRAM_ZI static tlsf_t    pxTlsf_Cust = NULL;
#endif
CUST_HEAP_6_RAMCODE void *pvPortZeroMallocCust( size_t xWantedSize)
{
    void *ptr = pvPortMalloc_CUST(xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
    return ptr ? memset(ptr, 0, xWantedSize), ptr : ptr;
}

CUST_HEAP_6_RAMCODE void *pvPortAssertMallocCust( size_t xWantedSize)
{
    void *ptr = pvPortMalloc_CUST(xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
    configASSERT(ptr != 0);
    return ptr;
}


CUST_HEAP_6_RAMCODE void *pvPortZeroAssertMallocCust( size_t xWantedSize)
{
    void *ptr = pvPortMalloc_CUST(xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
    configASSERT(ptr != 0);
    memset(ptr, 0, xWantedSize);
    return ptr;
}

CUST_HEAP_6_RAMCODE void *pvPortMalloc_CUST( size_t xWantedSize, unsigned int funcPtr )
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
        if(NULL == pxTlsf_Cust)
        {
            prvHeapInitCust();

        #ifdef MM_DEBUG_EN
            mm_trace_init_cust();
        #endif
        }
    #ifdef MM_DEBUG_EN
        if(funcPtr == 0)
        {
            funcPtr = (unsigned int)__GET_RETURN_ADDRESS();
        }
    #endif
        pvReturn = tlsf_malloc(pxTlsf_Cust, xWantedSize, (size_t)funcPtr);
    #ifdef MM_DEBUG_EN
        if( pvReturn != NULL )
        {
            mm_malloc_trace_cust(pvReturn, xWantedSize, funcPtr);
        }
    #endif
    }
    xTaskResumeAll();

    configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );

    return pvReturn;
}

CUST_HEAP_6_RAMCODE void *pvPortRealloc_CUST( void *pv, size_t xWantedSize,  unsigned int funcPtr )
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
        if(NULL == pxTlsf_Cust)
        {
            prvHeapInitCust();
        }
    #ifdef MM_DEBUG_EN
        if(funcPtr == 0)
        {
            funcPtr = (unsigned int)__GET_RETURN_ADDRESS();
        }
    #endif
        pvReturn = tlsf_realloc(pxTlsf_Cust, pv, xWantedSize, funcPtr);
    #ifdef MM_DEBUG_EN
        if( pvReturn != NULL )
        {
            mm_malloc_trace_cust(pvReturn, xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
        }
    #endif
    }
    xTaskResumeAll();

    configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );

    return pvReturn;
}

CUST_HEAP_6_RAMCODE void  vPortFreeCust( void *pv )
{
    configASSERT(__get_IPSR() == 0 && "no invokation by IPSR!");

    vTaskSuspendAll();
    {
        if(pxTlsf_Cust && pv)
        {
            tlsf_free(pxTlsf_Cust, pv);

        #ifdef MM_DEBUG_EN
            mm_free_trace_cust(pv);
        #endif
        }
    }
    xTaskResumeAll();
}

CUST_HEAP_6_RAMCODE size_t xPortGetTotalHeapSizeCust( void )
{
    return gTotalHeapSize_Cust;
}

CUST_HEAP_6_RAMCODE size_t xPortGetFreeHeapSizeCust( void )
{
    if(!pxTlsf_Cust) return 0;

    uint32_t mask = SaveAndSetIRQMask();
    gucPoolGroupSel = 1;
    size_t size = tlsf_mem_size_free(pxTlsf_Cust);
    gucPoolGroupSel = 0;
    RestoreIRQMask(mask);

    return size;
}

CUST_HEAP_6_RAMCODE uint8_t xPortGetFreeHeapPctCust( void )
{
    if(!pxTlsf_Cust) return 0;

    return (uint8_t)((xPortGetFreeHeapSizeCust() * 100) / xPortGetTotalHeapSizeCust());
}

CUST_HEAP_6_RAMCODE size_t xPortGetMaximumFreeBlockSizeCust( void )
{
    if(!pxTlsf_Cust) return 0;

    uint32_t mask = SaveAndSetIRQMask();

    size_t size = tlsf_mem_max_block_size(pxTlsf_Cust);

    RestoreIRQMask(mask);

    return size;
}

#define portHEAP_TOTAL_FREE_ALERT_PCT_CUST   30
#define portHEAP_FREE_BLOCK_ALERT_SIZE_CUST  8192

CUST_HEAP_6_RAMCODE uint8_t xPortIsFreeHeapOnAlertCust( void )
{
    return ((xPortGetFreeHeapPctCust() <= portHEAP_TOTAL_FREE_ALERT_PCT_CUST) || \
            (xPortGetMaximumFreeBlockSizeCust() <= portHEAP_FREE_BLOCK_ALERT_SIZE_CUST)) ? 1 : 0;
}

CUST_HEAP_6_RAMCODE size_t xPortGetMinimumEverFreeHeapSizeCust( void )
{
    if(!pxTlsf_Cust) return 0;

    uint32_t mask = SaveAndSetIRQMask();
    gucPoolGroupSel = 1;
    size_t size = tlsf_mem_size_ever_min(pxTlsf_Cust);
    gucPoolGroupSel = 0;
    RestoreIRQMask(mask);

    return size;
}

CUST_HEAP_6_RAMCODE void  vPortShowMemRecordCust( void )
{
    if(!pxTlsf_Cust)
    {
        return;
    }
    gucPoolGroupSel = 1;
    tlsf_show_mem_record(pxTlsf_Cust);
    gucPoolGroupSel = 0;
}

CUST_HEAP_6_RAMCODE void  vPortClearMemRecordCust( void )
{
    if(!pxTlsf_Cust)
    {
        return;
    }
    tlsf_clear_mem_record(pxTlsf_Cust);
}

CUST_HEAP_6_RAMCODE void  vPortShowPhysMemBlockCust(void *callback, int type, int *mem_range)
{
    int block_type;
    if(pxTlsf_Cust)
    {
        if(type <= 2)
        {
            tlsf_set_print_callback(callback);
            block_type = type;
            tlsf_check_pool(tlsf_get_pool(pxTlsf_Cust), block_type);
            tlsf_show_block_detail(pxTlsf_Cust);
            tlsf_show_hist_min_max_free_block(pxTlsf_Cust, NULL);
            tlsf_show_cur_max_block(pxTlsf_Cust, NULL);
        }
    }
}

CUST_HEAP_6_RAMCODE bool vPortGetHeapInfoCust(uint8_t type, int *mem_range)
{
    if(pxTlsf_Cust != NULL)
    {
        tlsf_set_print_callback(NULL);
        if(type == 0)
        {
            tlsf_show_hist_min_max_free_block(pxTlsf_Cust, mem_range);
        }
        else if(type == 1)
        {
            tlsf_show_cur_max_block(pxTlsf_Cust, mem_range);
        }
        return true;
    }
    return false;
}

CUST_HEAP_6_RAMCODE static void prvHeapInitCust( void )
{
    uint8_t   *aligned = (uint8_t*)portBYTE_ALIGN_UP((UBaseType_t)ucHeap_Cust, portBYTE_ALIGNMENT);

#ifdef CORE_IS_AP
    gTotalHeapSize_Cust = (UINT32)&(heap_endAddr_psram) - (UINT32)&(end_ap_data_psram);
    gTotalHeapSize_Cust = (gTotalHeapSize_Cust>TLSF_AP_HEAP_MAX_CUST) ? TLSF_AP_HEAP_MAX_CUST : gTotalHeapSize_Cust;
#else
    gTotalHeapSize_Cust = configTOTAL_HEAP_SIZE;
#endif
    gTotalHeapSize_Cust -= (aligned - ucHeap_Cust);

    gucPoolGroupSel = 1;
    pxTlsf_Cust = tlsf_create_with_pool(aligned, gTotalHeapSize_Cust);
    gucPoolGroupSel = 0;
}

#ifdef __USER_CODE__
extern void tlsf_mem_get_record(tlsf_t tlsf, uint32_t *alloc, uint32_t *peak);
CUST_HEAP_6_RAMCODE void GetPSRAMHeapInfo(uint32_t *total, uint32_t *alloc, uint32_t *peak)
{
	vTaskSuspendAll();
    if(NULL == pxTlsf_Cust)
    {
    	prvHeapInitCust();
    }
	*total = gTotalHeapSize_Cust;
	tlsf_mem_get_record(pxTlsf_Cust, alloc, peak);
	xTaskResumeAll();

}
#endif

#endif
#endif
#endif
