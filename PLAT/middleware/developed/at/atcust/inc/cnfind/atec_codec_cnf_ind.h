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
#ifndef __ATEC_CODEC_CNF_IND_H__
#define __ATEC_CODEC_CNF_IND_H__

#include "at_util.h"

CmsRetId ecCodecCnf(UINT16 reqHandle, UINT16 rc, void *paras);
void atApplCodecProcCmsCnf(CmsApplCnf *pCmsCnf);

#endif

/* END OF FILE */
