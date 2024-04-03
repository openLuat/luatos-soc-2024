/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    atec_ims_cnf_ind.h
 * Description:  API interface implementation source file for ims AT service
 * History:      Rev1.0   2018-10-12
 *
 ****************************************************************************/

#ifndef ATEC_IMS_CNF_IND_H
#define ATEC_IMS_CNF_IND_H

#include "imicomm.h"

/**
  \fn atImsProcImiCnf(ImaImiCnf *pImiCnf)
  \brief  process ims related at cnf sig
  \Note
*/
void atImsProcImiCnf(ImaImiCnf *pImiCnf);


/**
  \fn atImsProcImiInd(ImaImiInd *pImiInd)
  \brief  process ims related at ind sig
  \Note
*/
void atImsProcImiInd(ImaImiInd *pImiInd);

#endif
