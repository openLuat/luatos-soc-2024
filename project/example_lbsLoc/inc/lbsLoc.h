#ifndef _LBSLOC_H
#define _LBSLOC_H

#include "commontypedef.h"

#define AM_LOCATION_SERVICE_LOCATION_BCD_LEN 5

#define LBSLOC_SUCCESS          0x0  
#define UNKNOWN_LOCATION        0x1
#define PERMISSION_ERROR        0x2
#define UNKNOWN_ERROR           0x4
#define WIFILOC_SUCCESS         0xff

#ifdef PACK_STRUCT_USE_INCLUDES
#include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN
struct am_location_service_rsp_data_t
{
    PACK_STRUCT_FLD_8(u8_t result);
    PACK_STRUCT_FIELD(u8_t latitude[AM_LOCATION_SERVICE_LOCATION_BCD_LEN]);
    PACK_STRUCT_FIELD(u8_t longitude[AM_LOCATION_SERVICE_LOCATION_BCD_LEN]);
    PACK_STRUCT_FLD_8(u8_t year);
    PACK_STRUCT_FLD_8(u8_t month);
    PACK_STRUCT_FLD_8(u8_t day);
    PACK_STRUCT_FLD_8(u8_t hour);
    PACK_STRUCT_FLD_8(u8_t minute);
    PACK_STRUCT_FLD_8(u8_t second);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#include "arch/epstruct.h"
#endif



void lbsLoc_request(void);







#endif