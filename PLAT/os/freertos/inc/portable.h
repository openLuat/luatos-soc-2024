/*
 * FreeRTOS Kernel V9.0.0a
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------
 * Portable layer API.  Each function must be defined for each port.
 *----------------------------------------------------------*/

#ifndef PORTABLE_H
#define PORTABLE_H

/* Each FreeRTOS port has a unique portmacro.h header file.  Originally a
pre-processor definition was used to ensure the pre-processor found the correct
portmacro.h file for the port being used.  That scheme was deprecated in favour
of setting the compiler's include path such that it found the correct
portmacro.h file - removing the need for the constant and allowing the
portmacro.h file to be located anywhere in relation to the port being used.
Purely for reasons of backward compatibility the old method is still valid, but
to make it clear that new projects should not use it, support for the port
specific constants has been moved into the deprecated_definitions.h header
file. */
#include "deprecated_definitions.h"

/* If portENTER_CRITICAL is not defined then including deprecated_definitions.h
did not result in a portmacro.h header file being included - and it should be
included here.  In this case the path to the correct portmacro.h header file
must be set in the compiler's include path. */
#ifndef portENTER_CRITICAL
	#include "portmacro.h"
#endif

#if portBYTE_ALIGNMENT == 32
	#define portBYTE_ALIGNMENT_MASK ( 0x001f )
#endif

#if portBYTE_ALIGNMENT == 16
	#define portBYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if portBYTE_ALIGNMENT == 8
	#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if portBYTE_ALIGNMENT == 4
	#define portBYTE_ALIGNMENT_MASK	( 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
	#define portBYTE_ALIGNMENT_MASK	( 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
	#define portBYTE_ALIGNMENT_MASK	( 0x0000 )
#endif

#ifndef portBYTE_ALIGNMENT_MASK
	#error "Invalid portBYTE_ALIGNMENT definition"
#endif

#ifndef portNUM_CONFIGURABLE_REGIONS
	#define portNUM_CONFIGURABLE_REGIONS 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "mpu_wrappers.h"

/*
 * Setup the stack of a new task so it is ready to be placed under the
 * scheduler control.  The registers have to be placed on the stack in
 * the order that the port expects to find them.
 *
 */
#if( portUSING_MPU_WRAPPERS == 1 )
	StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters, BaseType_t xRunPrivileged ) PRIVILEGED_FUNCTION;
#else
	StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters ) PRIVILEGED_FUNCTION;
#endif

/* Used by heap_5.c. */
typedef struct HeapRegion
{
	uint8_t *pucStartAddress;
	size_t xSizeInBytes;
} HeapRegion_t;

/*
 * Used to define multiple heap regions for use by heap_5.c.  This function
 * must be called before any calls to pvPortMalloc() - not creating a task,
 * queue, semaphore, mutex, software timer, event group, etc. will result in
 * pvPortMalloc being called.
 *
 * pxHeapRegions passes in an array of HeapRegion_t structures - each of which
 * defines a region of memory that can be used as the heap.  The array is
 * terminated by a HeapRegions_t structure that has a size of 0.  The region
 * with the lowest start address must appear first in the array.
 */
void vPortDefineHeapRegions( const HeapRegion_t * const pxHeapRegions ) PRIVILEGED_FUNCTION;


void *pvPortMalloc_EC( size_t xWantedSize, unsigned int funcPtr );
void *pvPortRealloc_EC( void *pv, size_t xWantedSize,  unsigned int funcPtr );
void *pvPortZeroMallocEc( size_t xWantedSize);
void *pvPortAssertMallocEc( size_t xWantedSize);
void *pvPortZeroAssertMallocEc( size_t xWantedSize);
void  vPortFreeEc( void *pv );
size_t xPortGetTotalHeapSizeEc( void );
size_t xPortGetFreeHeapSizeEc( void );
size_t xPortGetMinimumEverFreeHeapSizeEc( void );
size_t xPortGetMaximumFreeBlockSizeEc( void );
uint8_t xPortGetFreeHeapPctEc( void );
uint8_t xPortIsFreeHeapOnAlertEc( void );

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1) || (defined TYPE_EC718M)
void *pvPortZeroMallocCust( size_t xWantedSize) PRIVILEGED_FUNCTION;
void *pvPortAssertMallocCust( size_t xWantedSize) PRIVILEGED_FUNCTION;
void *pvPortZeroAssertMallocCust( size_t xWantedSize) PRIVILEGED_FUNCTION;
void *pvPortMalloc_CUST( size_t xWantedSize, unsigned int funcPtr ) PRIVILEGED_FUNCTION;
void *pvPortRealloc_CUST( void *pv, size_t xWantedSize, unsigned int funcPtr ) PRIVILEGED_FUNCTION;
void vPortFreeCust( void *pv ) PRIVILEGED_FUNCTION;
size_t xPortGetTotalHeapSizeCust( void ) PRIVILEGED_FUNCTION;
size_t xPortGetFreeHeapSizeCust( void ) PRIVILEGED_FUNCTION;
size_t xPortGetMinimumEverFreeHeapSizeCust( void ) PRIVILEGED_FUNCTION;
size_t xPortGetMaximumFreeBlockSizeCust( void ) PRIVILEGED_FUNCTION;
uint8_t xPortGetFreeHeapPctCust( void ) PRIVILEGED_FUNCTION;
uint8_t xPortIsFreeHeapOnAlertCust( void ) PRIVILEGED_FUNCTION;
#endif


/*
 * Map to the memory management routines required for the port.
 */
#define pvPortMallocEc(xWantedSize)                  pvPortMalloc_EC(xWantedSize, 0)
#define pvPortReallocEc(pv, xWantedSize)             pvPortRealloc_EC(pv, xWantedSize, 0)

#ifdef TYPE_EC718M

#define pvPortMalloc(xWantedSize)                  pvPortMalloc_CUST(xWantedSize, 0)
#define pvPortRealloc(pv, xWantedSize)             pvPortRealloc_CUST(pv, xWantedSize, 0)
#define pvPortZeroMalloc(xWantedSize)              pvPortZeroMallocCust(xWantedSize)
#define pvPortAssertMalloc(xWantedSize)            pvPortAssertMallocCust(xWantedSize)
#define pvPortZeroAssertMalloc(xWantedSize)        pvPortZeroAssertMallocCust(xWantedSize)
#define vPortFree(pv )                             vPortFreeCust(pv ) 
#define vPortInitialiseBlocks()
#define xPortGetTotalHeapSize()                    xPortGetTotalHeapSizeCust()
#define xPortGetFreeHeapSize()                     xPortGetFreeHeapSizeCust()
#define xPortGetMinimumEverFreeHeapSize()          xPortGetMinimumEverFreeHeapSizeCust()
#define xPortGetMaximumFreeBlockSize()             xPortGetMaximumFreeBlockSizeCust()
#define xPortGetFreeHeapPct()                      xPortGetFreeHeapPctCust()
#define xPortIsFreeHeapOnAlert()                   xPortIsFreeHeapOnAlertCust()

#else //#ifdef TYPE_EC718M

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
#define pvPortMallocCust(xWantedSize)               pvPortMalloc_CUST(xWantedSize, 0)
#define pvPortReallocCust(pv, xWantedSize)          pvPortRealloc_CUST(pv, xWantedSize, 0)

#define pvPortMalloc_Psram(xWantedSize)             pvPortMalloc_CUST(xWantedSize, 0)
#define pvPortRealloc_Psram(pv, xWantedSize)        pvPortRealloc_CUST(pv, xWantedSize, 0)
#define pvPortZeroMalloc_Psram(xWantedSize)         pvPortZeroMallocCust(xWantedSize)
#define pvPortZeroAssertMalloc_Psram(xWantedSize)   pvPortZeroAssertMallocCust(xWantedSize)
#define vPortFree_Psram(pv)                         vPortFreeCust(pv )
#define xPortGetTotalHeapSize_Psram()               xPortGetTotalHeapSizeCust()
#ifdef TYPE_EC718U
#else
#define pvPortAssertMalloc_Psram(xWantedSize)       pvPortAssertMallocCust(xWantedSize)
#define xPortGetFreeHeapSize_Psram()                xPortGetFreeHeapSizeCust()
#endif
#define xPortGetMinimumEverFreeHeapSize_Psram()     xPortGetMaximumFreeBlockSizeCust()
#define xPortGetMaximumFreeBlockSize_Psram()        xPortGetMaximumFreeBlockSizeCust()
#define xPortGetFreeHeapPct_Psram()                 xPortGetFreeHeapPctCust()
#define xPortIsFreeHeapOnAlert_Psram()              xPortIsFreeHeapOnAlertCust()

#endif

#endif

#if (defined __USER_CODE__) && (defined TYPE_EC718M)
#undef pvPortMalloc
#undef pvPortRealloc
#undef pvPortZeroMalloc
#undef pvPortAssertMalloc
#undef pvPortZeroAssertMalloc
#undef vPortFree
#undef vPortInitialiseBlocks
#undef xPortGetTotalHeapSize
#undef xPortGetFreeHeapSize
#undef xPortGetMinimumEverFreeHeapSize
#undef xPortGetMaximumFreeBlockSize
#undef xPortGetFreeHeapPct
#undef xPortIsFreeHeapOnAlert

#define pvPortMalloc(xWantedSize)                  pvPortMalloc_EC(xWantedSize, 0)
#define pvPortRealloc(pv, xWantedSize)             pvPortRealloc_EC(pv, xWantedSize, 0)

#define pvPortZeroMalloc(xWantedSize)              pvPortZeroMallocEc(xWantedSize)
#define pvPortAssertMalloc(xWantedSize)            pvPortAssertMallocEc(xWantedSize)
#define pvPortZeroAssertMalloc(xWantedSize)        pvPortZeroAssertMallocEc(xWantedSize)
#define vPortFree(pv )                             vPortFreeEc(pv )
#define xPortGetTotalHeapSize()                    xPortGetTotalHeapSizeEc()
#define xPortGetMinimumEverFreeHeapSize()          xPortGetMinimumEverFreeHeapSizeEc()
#define xPortGetMaximumFreeBlockSize()             xPortGetMaximumFreeBlockSizeEc()
#define xPortGetFreeHeapSize()                     xPortGetFreeHeapSizeEc()

#define vPortInitialiseBlocks()
#define xPortGetFreeHeapPct()                      xPortGetFreeHeapPctEc()
#define xPortIsFreeHeapOnAlert()                   xPortIsFreeHeapOnAlertEc()
#endif

#if (defined __USER_CODE__) && (defined TYPE_EC718U)
#undef vPortFree_Psram
#endif
/*
 * Setup the hardware ready for the scheduler to take control.  This generally
 * sets up a tick interrupt and sets timers for the correct tick frequency.
 */
BaseType_t xPortStartScheduler( void ) PRIVILEGED_FUNCTION;

/*
 * Undo any hardware/ISR setup that was performed by xPortStartScheduler() so
 * the hardware is left in its original condition after the scheduler stops
 * executing.
 */
void vPortEndScheduler( void ) PRIVILEGED_FUNCTION;

/*
 * The structures and methods of manipulating the MPU are contained within the
 * port layer.
 *
 * Fills the xMPUSettings structure with the memory region information
 * contained in xRegions.
 */
#if( portUSING_MPU_WRAPPERS == 1 )
	struct xMEMORY_REGION;
	void vPortStoreTaskMPUSettings( xMPU_SETTINGS *xMPUSettings, const struct xMEMORY_REGION * const xRegions, StackType_t *pxBottomOfStack, uint32_t ulStackDepth ) PRIVILEGED_FUNCTION;
#endif

#ifdef __cplusplus
}
#endif

#endif /* PORTABLE_H */

