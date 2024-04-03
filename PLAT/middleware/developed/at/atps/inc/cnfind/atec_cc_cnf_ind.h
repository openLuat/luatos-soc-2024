/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    atec_cc_cnf_ind.h
 * Description:  API interface implementation source file for ims AT service
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/

#ifndef ATEC_CC_CNF_IND_H
#define ATEC_CC_CNF_IND_H

#include "imicomm.h"

/**
  \fn atCcProcImiCnf(ImaImiCnf *pImiCnf)
  \brief  process ims related at cnf sig
  \Note
*/
void atCcProcImiCnf(ImaImiCnf *pImiCnf);


/**
  \fn atCcProcImiInd(ImaImiInd *pImiInd)
  \brief  process ims related at ind sig
  \Note
*/
void atCcProcImiInd(ImaImiInd *pImiInd);

#endif
