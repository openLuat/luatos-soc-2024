/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef OS_COMMON_H
#define OS_COMMON_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "cmsis_compiler.h"
#include "exception_process.h"

#include DEBUG_LOG_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/


#define OsaCheck(cond, v1, v2, v3)  \
    EC_ASSERT((BOOL)(cond), (UINT32)(v1), (UINT32)(v2), (UINT32)(v3))

/*
 * OSA memory function
*/
//void* OsaAllocMemory(UINT32 size);
#define     OsaAllocMemory(S)           pvPortAssertMalloc((S))         /* Assert, if no memory allocated */
//void* OsaAllocZeroMemory(UINT32 size);
#define     OsaAllocZeroMemory(S)       pvPortZeroAssertMalloc((S))     /* Assert, if no memory allocated */
//void* OsaAllocMemoryNoAssert(UINT32 size);
#define     OsaAllocMemoryNoAssert(S)   pvPortMalloc((S))               /* return PNULL, if no memory allcated */
//void* OsaAllocZeroMemoryNoAssert(UINT32 size);
#define     OsaAllocZeroMemoryNoAssert(S)       pvPortZeroMalloc((S))   /* return PNULL, if no memory allcated */
//void* OsaReallocMemory(void *pv, UINT32 size);
#define     OsaReallocMemory(pv, S)     pvPortRealloc((pv), (S))


#define OsaFreeMemory(pPtr)     \
do {                            \
    vPortFree(*(pPtr));                                                     \
    (*(pPtr)) = PNULL;                                                      \
}while(FALSE);

#define OsaFreeMemoryNoCheck(pPtr)      \
do {                                    \
    vPortFree(*(pPtr));                                                     \
    (*(pPtr)) = PNULL;                                                      \
}while(FALSE)

#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
//void* OsaAllocMemory_Psram(UINT32 size);
#define     OsaAllocMemory_Psram(S)        pvPortAssertMalloc_Psram((S))         /* Assert, if no memory allocated */
//void* OsaAllocZeroMemory_Psram(UINT32 size);
#define     OsaAllocZeroMemory_Psram(S)    pvPortZeroAssertMalloc_Psram((S))     /* Assert, if no memory allocated */
//void* OsaAllocMemoryNoAssert_Psram(UINT32 size);
#define     OsaAllocMemoryNoAssert_Psram(S)      pvPortMalloc_Psram((S))         /* return PNULL, if no memory allcated */
//void* OsaAllocZeroMemoryNoAssert_Psram(UINT32 size);
#define     OsaAllocZeroMemoryNoAssert_Psram(S)       pvPortZeroMalloc_Psram((S))   /* return PNULL, if no memory allcated */


#define OsaFreeMemory_Psram(pPtr)     \
do {                            \
    vPortFree_Psram(*(pPtr));                                                     \
    (*(pPtr)) = PNULL;                                                      \
}while(FALSE);

#endif

/* !!! Del do while. Otherwise, use continue/break between OsaDebugBegin and OsaDebugEnd,
   will invaild and cause ambiguity !!! */
#if defined (TYPE_EC718S) || defined (TYPE_EC716S)
#define OsaDebugBegin(cond, v1, v2, V3)     \
if (!(cond))                                \
{                                           \
    ECPLAT_PRINTF(UNILOG_OSA, OsaCheckDebugFalse_e_1, P_ERROR, "Debug Error, file: %s, line: %d, (0x%x, 0x%x)", (UINT8 *)_STRINGNIFY_(__CURRENT_FILE_NAME__), __LINE__, (UINT32)(v1), (UINT32)(v2));

#define OsaDebugEnd()                       \
}
#else
#define OsaDebugBegin(cond, v1, v2, v3)     \
if (!(cond))                                \
{                                           \
    ECPLAT_PRINTF(UNILOG_OSA, OsaCheckDebugFalse_e_1, P_ERROR, "Debug Error, func: %s, line: %d, (0x%x, 0x%x, 0x%x)", __FUNCTION__, __LINE__, (UINT32)(v1), (UINT32)(v2), (UINT32)(v3));

#define OsaDebugEnd()                       \
}
#endif



/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/







/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/




#ifdef __cplusplus
}
#endif

#endif
