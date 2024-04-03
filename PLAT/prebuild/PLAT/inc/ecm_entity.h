/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ecm_entity.h
*
*  Description:
*
*  History: 2022/2/22 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef ECM_ENTITY_H
#define ECM_ENTITY_H

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

typedef EtherEntity_t EcmEntity_t;

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
 * @brief ceioInitEntity(SerialEntity_t *serlEnt, chentStatusCallback statusCb, void *extras)
 * @details create a ecm entity
 *
 * @param ecmEnt     The entity to be created
 * @param status_cb  The handler of entity status
 * @param extras The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ceioInitEntity(EcmEntity_t *ecmEnt,
                       chentStatusCallback statusCb,
                       void *extras);

/**
 * @brief ceioDeinitEntity(EcmEntity_t *ecmEnt)
 * @details delete/reset a ecm entity
 *
 * @param ecmEnt  The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ceioDeinitEntity(EcmEntity_t *ecmEnt);

/**
 * @brief ceioSetUpChannel(EcmEntity_t *ecmEnt)
 * @details establish a ecm channel
 *
 * @param ecmEnt  The entity to be established the channel
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ceioSetUpChannel(EcmEntity_t *ecmEnt);

/**
 * @brief ceioPullDownChannel(EcmEntity_t *ecmEnt)
 * @details destroy a ecm channel
 *
 * @param ecmEnt The entity to be destroied the channel
 * @return 0 succ; < 0 failure with errno.
 */
int32_t ceioPullDownChannel(EcmEntity_t *ecmEnt);


#ifdef __cplusplus
}
#endif
#endif








