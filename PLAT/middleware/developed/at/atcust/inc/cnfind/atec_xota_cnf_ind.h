/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:atec_xota_cnf_ind.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_XOTA_CNF_IND_H__
#define __ATEC_XOTA_CNF_IND_H__

#include "at_util.h"

void atApplFwupdProcCmsCnf(CmsApplCnf *pCmsCnf);
void atApplEcotaProcCmsCnf(CmsApplCnf *pCmsCnf);

void atApplXotaProcCmsInd(CmsApplInd *pCmsInd);

#endif

/* END OF FILE */
