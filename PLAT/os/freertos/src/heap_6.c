
#include "FreeRTOS.h"
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
#include "cmsis_os2.h"

#if( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
    #error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif




#ifdef CORE_IS_AP
    extern UINT32 start_up_buffer;
    extern UINT32 end_ap_data;
#ifdef __USER_CODE__
#ifdef TYPE_EC718M
    extern UINT32 heap_endAddr_psram;
    extern UINT32 end_ap_data_psram;
#endif
#endif
#endif


/* Allocate the memory for the heap. */
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
    /* The application writer has already defined the array used for the RTOS
     * heap - probably so it can be placed in a special segment or address.
     */
    extern uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#else
    //static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
    #ifdef CORE_IS_AP
#ifdef __USER_CODE__
#ifdef TYPE_EC718M
    AP_PLAT_COMMON_DATA uint8_t * ucHeap=(uint8_t *)&( end_ap_data_psram );
#else
    AP_PLAT_COMMON_DATA uint8_t * ucHeap=(uint8_t *)&( end_ap_data );
#endif
#else
    AP_PLAT_COMMON_DATA uint8_t * ucHeap=(uint8_t *)&( end_ap_data );
#endif
    #if MM_TRACE_ON == 2
    #define TLSF_AP_HEAP_MAX  (128*1024)        // worse case of heap size
    #else
    #define TLSF_AP_HEAP_MAX  tlsf_block_size_max()
    #endif
    #else
    __ALIGNED(8) static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];//cp still use fix length array
    #endif

    //dynamic heap size, caculate per compilation
AP_PLAT_COMMON_BSS UINT32 gTotalHeapSize=0;

#endif /* configAPPLICATION_ALLOCATED_HEAP */


/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInitEc( void );

AP_PLAT_COMMON_BSS static tlsf_t    pxTlsf = NULL;

FREERTOS_HEAP6_TEXT_SECTION void *pvPortZeroMallocEc( size_t xWantedSize)
{
    void *ptr = pvPortMalloc_EC(xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
    return ptr ? memset(ptr, 0, xWantedSize), ptr : ptr;
}

FREERTOS_HEAP6_TEXT_SECTION void *pvPortAssertMallocEc( size_t xWantedSize)
{
    void *ptr = pvPortMalloc_EC(xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
    configASSERT(ptr != 0);
    return ptr;
}

FREERTOS_HEAP6_TEXT_SECTION void *pvPortZeroAssertMallocEc( size_t xWantedSize)
{
    void *ptr = pvPortMalloc_EC(xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
    configASSERT(ptr != 0);
    memset(ptr, 0, xWantedSize);
    return ptr;
}

FREERTOS_HEAP6_TEXT_SECTION void *pvPortMalloc_EC( size_t xWantedSize, unsigned int funcPtr )
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
        if(NULL == pxTlsf)
        {
            prvHeapInitEc();

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
        pvReturn = tlsf_malloc(pxTlsf, xWantedSize, (size_t)funcPtr);
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

FREERTOS_HEAP6_TEXT_SECTION void *pvPortRealloc_EC( void *pv, size_t xWantedSize,  unsigned int funcPtr )
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
        if(NULL == pxTlsf)
        {
            prvHeapInitEc();
        }
    #ifdef MM_DEBUG_EN
        if(funcPtr == 0)
        {
            funcPtr = (unsigned int)__GET_RETURN_ADDRESS();
        }
    #endif
        pvReturn = tlsf_realloc(pxTlsf, pv, xWantedSize, funcPtr);
    #ifdef MM_DEBUG_EN
        if( pvReturn != NULL && pvReturn != pv)
        {
            mm_free_trace(pv);
            mm_malloc_trace(pvReturn, xWantedSize, (unsigned int)__GET_RETURN_ADDRESS());
        }
    #endif
    }
    xTaskResumeAll();

    configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );

    return pvReturn;
}

#ifdef __USER_CODE__
FREERTOS_HEAP6_TEXT_SECTION void *pvPortMemAlignMallocEC(size_t xWantedSize, size_t align)
{
    void *pvReturn = NULL;

    configASSERT(__get_IPSR() == 0 && "no invokation by IPSR!");
    configASSERT(xWantedSize > 0 && "zero alloc is prohibited!");

#ifdef MEM_BLK_SIZE_32BIT
    /* do nothing */
#else
    configASSERT(xWantedSize < 0x10000 && "64K(+) alloc is prohibited!");
#endif

    vTaskSuspendAll();
    {
        if(NULL == pxTlsf)
        {
            prvHeapInitEc();

        #ifdef MM_DEBUG_EN
            mm_trace_init();
        #endif
        }
    #ifdef MM_DEBUG_EN

        unsigned int funcPtr = (unsigned int)__GET_RETURN_ADDRESS();

    #endif
        pvReturn = tlsf_memalign(pxTlsf, align, xWantedSize, (size_t)funcPtr);
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
#endif

FREERTOS_HEAP6_TEXT_SECTION void  vPortFreeEc( void *pv )
{
    configASSERT(__get_IPSR() == 0 && "no invokation by IPSR!");

    vTaskSuspendAll();
    {
        if(pxTlsf && pv)
        {
            tlsf_free(pxTlsf, pv);

        #ifdef MM_DEBUG_EN
            mm_free_trace(pv);
        #endif
        }
    }
    xTaskResumeAll();
}

FREERTOS_HEAP6_TEXT_SECTION size_t xPortGetTotalHeapSizeEc( void )
{
    return gTotalHeapSize;
}

FREERTOS_HEAP6_TEXT_SECTION size_t xPortGetFreeHeapSizeEc( void )
{
    if(!pxTlsf) return 0;

    uint32_t mask = SaveAndSetIRQMask();

    size_t size = tlsf_mem_size_free(pxTlsf);

    RestoreIRQMask(mask);

    return size;
}

FREERTOS_HEAP6_TEXT_SECTION uint8_t xPortGetFreeHeapPctEc( void )
{
    if(!pxTlsf) return 0;

    return (uint8_t)((xPortGetFreeHeapSizeEc() * 100) / xPortGetTotalHeapSizeEc());
}

FREERTOS_HEAP6_TEXT_SECTION size_t xPortGetMaximumFreeBlockSizeEc( void )
{
    if(!pxTlsf) return 0;

    uint32_t mask = SaveAndSetIRQMask();

    size_t size = tlsf_mem_max_block_size(pxTlsf);

    RestoreIRQMask(mask);

    return size;
}

#define portHEAP_TOTAL_FREE_ALERT_PCT   30
#define portHEAP_FREE_BLOCK_ALERT_SIZE  8192
FREERTOS_HEAP6_TEXT_SECTION uint8_t xPortIsFreeHeapOnAlertEc( void )
{
    return ((xPortGetFreeHeapPctEc() <= portHEAP_TOTAL_FREE_ALERT_PCT) || \
            (xPortGetMaximumFreeBlockSizeEc() <= portHEAP_FREE_BLOCK_ALERT_SIZE)) ? 1 : 0;
}

FREERTOS_HEAP6_TEXT_SECTION size_t xPortGetMinimumEverFreeHeapSizeEc( void )
{
    if(!pxTlsf) return 0;

    uint32_t mask = SaveAndSetIRQMask();

    size_t size = tlsf_mem_size_ever_min(pxTlsf);

    RestoreIRQMask(mask);

    return size;
}

FREERTOS_HEAP6_TEXT_SECTION void  vPortShowMemRecordEc( void )
{
    if(!pxTlsf)
    {
        return;
    }
    tlsf_show_mem_record(pxTlsf);
}

FREERTOS_HEAP6_TEXT_SECTION void  vPortClearMemRecordEc( void )
{
    if(!pxTlsf)
    {
        return;
    }
    tlsf_clear_mem_record(pxTlsf);
}

FREERTOS_HEAP6_TEXT_SECTION void  vPortShowPhysMemBlockEc(void *callback, int type, int *mem_range)
{
    int block_type;
    if(pxTlsf)
    {
        if(type <= 2)
        {
            tlsf_set_print_callback(callback);
            block_type = type;
            tlsf_check_pool(tlsf_get_pool(pxTlsf), block_type);
            tlsf_show_block_detail(pxTlsf);
            tlsf_show_hist_min_max_free_block(pxTlsf, NULL);
            tlsf_show_cur_max_block(pxTlsf, NULL);
        }
    }
}

FREERTOS_HEAP6_TEXT_SECTION bool vPortGetHeapInfoEc(uint8_t type, int *mem_range)
{
    if(pxTlsf != NULL)
    {
        tlsf_set_print_callback(NULL);
        if(type == 0)
        {
            tlsf_show_hist_min_max_free_block(pxTlsf, mem_range);
        }
        else if(type == 1)
        {
            tlsf_show_cur_max_block(pxTlsf, mem_range);
        }
        return true;
    }
    return false;
}


FREERTOS_HEAP6_TEXT_SECTION static void prvHeapInitEc( void )
{
#ifdef __USER_CODE__
#else
    uint8_t   *aligned = (uint8_t*)portBYTE_ALIGN_UP((UBaseType_t)ucHeap, portBYTE_ALIGNMENT);
#endif

#ifdef __USER_CODE__
#ifdef TYPE_EC718M
	gTotalHeapSize = (UINT32)&(heap_endAddr_psram) - (UINT32)&(end_ap_data_psram);
#else
	gTotalHeapSize = (UINT32)&(start_up_buffer) - (UINT32)&(end_ap_data);
#endif
#else
#ifdef CORE_IS_AP
    gTotalHeapSize = (UINT32)&(start_up_buffer) - (UINT32)&(end_ap_data);
    gTotalHeapSize = (gTotalHeapSize>TLSF_AP_HEAP_MAX) ? TLSF_AP_HEAP_MAX : gTotalHeapSize;
#else
    gTotalHeapSize = configTOTAL_HEAP_SIZE;
#endif
#endif
#ifdef __USER_CODE__
    pxTlsf = tlsf_create_with_pool(ucHeap, gTotalHeapSize);
#else
    gTotalHeapSize -= (aligned - ucHeap);

    pxTlsf = tlsf_create_with_pool(aligned, gTotalHeapSize);
#endif
}

#ifdef __USER_CODE__
extern void tlsf_mem_get_record(tlsf_t tlsf, uint32_t *alloc, uint32_t *peak);
FREERTOS_HEAP6_TEXT_SECTION void GetSRAMHeapInfo(uint32_t *total, uint32_t *alloc, uint32_t *peak)
{
	vTaskSuspendAll();
	*total = gTotalHeapSize;
	tlsf_mem_get_record(pxTlsf, alloc, peak);
	xTaskResumeAll();
}

FREERTOS_HEAP6_TEXT_SECTION void *pvPortMalloc_CUST( size_t xWantedSize, unsigned int funcPtr )
{
	return pvPortMalloc_EC(xWantedSize, funcPtr);
}

FREERTOS_HEAP6_TEXT_SECTION void  vPortFreeCust( void *pv )
{
    if (pv != NULL)
        vPortFreeEc( pv ) ;
}

#ifdef TYPE_EC718U
FREERTOS_HEAP6_TEXT_SECTION bool vPortGetHeapInfo(uint8_t type, int *mem_range)
{
	return vPortGetHeapInfoEc(type, mem_range);
}

FREERTOS_HEAP6_TEXT_SECTION size_t xPortGetMinimumEverFreeHeapSize( void )
{
    return xPortGetMinimumEverFreeHeapSizeEc();
}

FREERTOS_HEAP6_TEXT_SECTION size_t xPortGetTotalHeapSize( void )
{
    return gTotalHeapSize;
}

FREERTOS_HEAP6_TEXT_SECTION size_t xPortGetMaximumFreeBlockSize( void )
{
    return xPortGetMaximumFreeBlockSizeEc();
}

FREERTOS_HEAP6_TEXT_SECTION void *pvPortZeroMalloc( size_t xWantedSize)
{
	return pvPortZeroMallocEc(xWantedSize);
}

FREERTOS_HEAP6_TEXT_SECTION void *pvPortAssertMalloc( size_t xWantedSize)
{
	return pvPortAssertMallocEc(xWantedSize);
}

FREERTOS_HEAP6_TEXT_SECTION void *pvPortZeroAssertMalloc( size_t xWantedSize)
{
	return pvPortZeroAssertMallocEc(xWantedSize);
}

FREERTOS_HEAP6_TEXT_SECTION void *pvPortMallocEC( size_t xWantedSize, unsigned int funcPtr )
{
	return pvPortMalloc_EC(xWantedSize, funcPtr);
}
FREERTOS_HEAP6_TEXT_SECTION void  vPortFree( void *pv )
{
    if (pv != NULL)
        vPortFreeEc( pv ) ;
}
FREERTOS_HEAP6_TEXT_SECTION size_t  xPortGetFreeHeapSize(void)
{
	return xPortGetFreeHeapSizeEc();
}
FREERTOS_HEAP6_TEXT_SECTION void *pvPortMallocEC_Psram( size_t xWantedSize, unsigned int funcPtr )
{
	return pvPortMalloc_EC(xWantedSize, funcPtr);
}

FREERTOS_HEAP6_TEXT_SECTION void  vPortFree_Psram( void *pv )
{
	vPortFreeEc(pv);
}

#endif
#endif


#if ( configUSE_NEWLIB_REENTRANT == 1 )
#include <reent.h>
void *__wrap__malloc_r(struct _reent*reent_ptr, size_t Size)
{
    void *ptr;

#if (defined TYPE_EC718M) && (defined CORE_IS_AP)
    ptr = pvPortMalloc(Size) ;
#else
    ptr = pvPortMallocEc(Size) ;
#endif
    //#ifdef MM_DEBUG_EN
    #if 0
    mm_malloc_trace(ptr, Size);
    #endif

    return ptr;
}
__attribute__((used)) void *__wrap__realloc_r(struct _reent*reent_ptr, void *pv, size_t xWantedSize)
{
    void *ptr;
    
#if (defined TYPE_EC718M) && (defined CORE_IS_AP)
    ptr = pvPortRealloc(pv, xWantedSize) ;
#else
    ptr = pvPortReallocEc(pv, xWantedSize) ;
#endif

    //#ifdef MM_DEBUG_EN
    #if 0
    mm_malloc_trace(ptr, Size);
    #endif

    return ptr;
}

void __wrap__free_r(struct _reent*reent_ptr, void *p)
{

    //#ifdef MM_DEBUG_EN
    #if 0
    mm_free_trace(p);
    #endif
    if (p != NULL)
#if (defined TYPE_EC718M) && (defined CORE_IS_AP)
    vPortFree(p) ;
#else
    vPortFreeEc(p) ;
#endif
}

#endif

#endif

