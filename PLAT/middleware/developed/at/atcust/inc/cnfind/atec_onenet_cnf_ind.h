/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_onenet_cnf_ind.h
*
*  Description: Process onenet related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_ONENET_CNF_IND_H
#define _ATEC_ONENET_CNF_IND_H

#include "at_util.h"

void atApplOnenetProcCmsCnf(CmsApplCnf *pCmsCnf);
void atApplOnenetProcCmsInd(CmsApplInd *pCmsInd);

#endif

