/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:atec_wifi_cnf_ind.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_WIFI_CNF_IND_H__
#define __ATEC_WIFI_CNF_IND_H__

#include "at_util.h"

void atApplEcWanSwitchConf(CmsApplCnf *pCmsCnf);

void atApplEcWanSwitchInd(CmsApplInd *pCmsInd);



#endif/*__ATEC_WIFI_CNF_IND_H__*/
