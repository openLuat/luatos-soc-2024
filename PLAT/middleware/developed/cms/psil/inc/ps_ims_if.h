/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    ps_ims_if.h
 * Description:  API interface implementation header file for IMS service
 * History:      Rev1.0
 *
 ****************************************************************************/
#ifndef __PS_IMS_IF_H__
#define __PS_IMS_IF_H__
#include "cms_util.h"
#include "imicomm.h"
#include "imireg.h"
#include "imipro.h"

/******************************************************************************
 * ASYN API, just send IMI REG sig to IMS
******************************************************************************/
CmsRetId imsSetECIMSREG(UINT16 atHandler, UINT8 regAct);
CmsRetId imsSetECIMSJSON(UINT16 atHandler, CHAR *fileName, UINT8 oper, CHAR *path, CHAR *value);
CmsRetId imsGetJsonFileList(UINT16 atHandler);
CmsRetId imsReadUsageSetting(UINT16 atHandler);



/******************************************************************************
 * SYNC API
******************************************************************************/
ImiRcCode imsGetECIMSREGSync(UINT8 *pImsRegState);

ImiRcCode imsGetCIREGSync(UINT8 *pImsRegState,UINT32 *regExtInfo);


#endif

