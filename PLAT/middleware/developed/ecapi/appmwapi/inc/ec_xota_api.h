/******************************************************************************
 * (C) Copyright 2018 AirM2M International Ltd.
 * All Rights Reserved
*******************************************************************************
 *  Filename: ec_xota_api.h
 *
 *  Description: API interface implementation header file for adc service
 *
 *  History:
 *
 *  Notes:
 *
******************************************************************************/
#ifndef __EC_XOTA_API_H__
#define __EC_XOTA_API_H__

typedef enum
{
    XOTA_EC_PARAM_INVALID = 1,
    XOTA_EC_OPER_UNSUPP,
    XOTA_EC_PKGSZ_ERROR,
    XOTA_EC_PKGSN_ERROR,
    XOTA_EC_CRC8_ERROR,
    XOTA_EC_FLERASE_UNDONE,
    XOTA_EC_FLERASE_ERROR,
    XOTA_EC_FLWRITE_ERROR,
    XOTA_EC_FLREAD_ERROR,
    XOTA_EC_UNDEF_ERROR
}AtXotaErrCode_e;


#endif

