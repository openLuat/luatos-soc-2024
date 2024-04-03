/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: rndis_entity.h
*
*  Description:
*
*  History: 2021/1/19 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef RNDIS_ENTITY_H
#define RNDIS_ENTITY_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

typedef EtherEntity_t RndisEntity_t;

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
 * @brief crioInitEntity(SerialEntity_t *serlEnt, chentStatusCallback statusCb, void *extras)
 * @details create a rndis entity
 *
 * @param rndisEnt   The entity to be created
 * @param status_cb  The handler of entity status
 * @param extras The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t crioInitEntity(RndisEntity_t *rndisEnt,
                       chentStatusCallback statusCb,
                       void *extras);

/**
 * @brief crioDeinitEntity(RndisEntity_t *rndisEnt)
 * @details delete/reset a rndis entity
 *
 * @param rndisEnt The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t crioDeinitEntity(RndisEntity_t *rndisEnt);

/**
 * @brief crioSetUpChannel(RndisEntity_t *rndisEnt)
 * @details establish a rndis channel
 *
 * @param rndisEnt The entity to be established the channel
 * @return 0 succ; < 0 failure with errno.
 */
int32_t crioSetUpChannel(RndisEntity_t *rndisEnt);

/**
 * @brief crioPullDownChannel(RndisEntity_t *rndisEnt)
 * @details destroy a rndis channel
 *
 * @param rndisEnt The entity to be destroied the channel
 * @return 0 succ; < 0 failure with errno.
 */
int32_t crioPullDownChannel(RndisEntity_t *rndisEnt);


#ifdef __cplusplus
}
#endif
#endif








