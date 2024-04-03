/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: eutra_entity.h
*
*  Description:
*
*  History: 2021/1/19 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef CCID_ENTITY_H
#define CCID_ENTITY_H

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

typedef struct
{
    /* inherited field & MUST be placed on the top! */
    CcioEntity_t  base;
    uint8_t  rsvd[4];

    void  *extras;              /* for user context. */
}CcidEntity_t;



/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
 * @brief cuioInitEntity(EutraEntity_t *eutraEnt, struct netif *eutraIf, chentStatusCallback statusCb, void *extras)
 * @details create a rndis entity
 *
 * @param eutraEnt The entity to be created
 * @param eutraIf The eutra(ps) netif info
 * @param status_cb The handler of entity status
 * @param extras The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t cdioInitEntity(CcidEntity_t *ccidEnt,
                       chentStatusCallback statusCb,
                       void *extras);

/**
 * @brief cuioDeinitEntity(EutraEntity_t *eutraEnt)
 * @details delete/reset a rndis entity
 *
 * @param eutraEnt The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t cdioDeinitEntity(CcidEntity_t *ccidEnt);

/**
 * @brief cuioSetUpChannel(EutraEntity_t *eutraEnt)
 * @details establish a eutra channel
 *
 * @param eutraEnt The entity to be established the channel
 * @return 0 succ; < 0 failure with errno.
 */
int32_t cdioSetUpChannel(CcidEntity_t *ccidEnt);

/**
 * @brief cuioPullDownChannel(EutraEntity_t *eutraEnt)
 * @details destroy a eutra channel
 *
 * @param eutraEnt The entity to be destroied the channel
 * @return 0 succ; < 0 failure with errno.
 */
int32_t cdioPullDownChannel(CcidEntity_t *ccidEnt);


#ifdef __cplusplus
}
#endif
#endif

