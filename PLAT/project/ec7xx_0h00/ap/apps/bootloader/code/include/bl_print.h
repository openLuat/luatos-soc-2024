#ifndef  BL_PRINT_H
#define  BL_PRINT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ec7xx.h"
#include "bsp.h"
#include <stdio.h>

typedef enum
{
    EC_NUM_BASE_10 = 10,
    EC_NUM_BASE_16 = 16,
}EC_NumBase_E;

#define EC_PRINTF_BUFFER_LENGTH 256

/*
 *  EC_Printf function.
 *  - Support output format:
 *    %x,%X,%s,%S,%d,%D. example likes EC_Printf("0x%x,%d,%s",10,10,"example").
 *  - return value:
 *    return the length of the outputed data if success;
 *    return -1 if failure;
 *  - note: 
 *    double characters '%' can output one character '%'.
 */
int EC_Printf(const char * pFormat, ...);

/*
 *  EC_Sprintf function.
 *  - Support format: 
 *    %x,%X,%02x,%02X(0:filled value, 2:length),%s,%S,%d,%D.
 *    example likes EC_Sprintf(pBuf,"0x%x,%d,%s,0x%08x",10,10,"example",10).
 *  - return value:
 *    return the length of the outputed data if success;
 *    return -1 if failure;
 */
int EC_Sprintf(char *pBuf, const char * pFormat, ...);

/*
 *  EC_Snprintf function.
 *  - Support output format: 
 *    %x,%X,%02x,%02X,%s,%S,%d,%D.
 *    example likes EC_Snprintf(pBuf,20,"0x%x,%d,%s",10,10,"example").
 *  - return value:
 *    return the length of the spliced data if success;
 *    return -1 if failure;
 */
int EC_Snprintf(char *pBuf, int size, const char * pFormat, ...);

#ifdef __cplusplus
}
#endif

#endif /* BL_PRINT_H */
