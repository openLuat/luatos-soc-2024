/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ctiot_cnf_ind_api.h
*
*  Description: Process ctiot related AT commands header
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_CTIOT_CNF_IND_API_H
#define _ATEC_CTIOT_CNF_IND_API_H

#include "at_util.h"

typedef struct
{
	int ret;
	int atHandle;
}ctlwm2m_cnf_msg;


void atApplCtProcCmsCnf(CmsApplCnf *pCmsCnf);
void atApplCtProcCmsInd(CmsApplInd *pCmsInd);

#endif

