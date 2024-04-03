/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    cache.h
 * Description:  EC718 cache controller driver header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/

#ifndef _CACHE_EC7XX_H
#define _CACHE_EC7XX_H
#include "Driver_Common.h"

 /**
   \addtogroup icache_interface_gr
   \{
  */

#ifdef __cplusplus
 extern "C" {
#endif

/**
  \fn    void EnableICache(void)
  \brief Enables Instrution cache
  \return void
 */
void EnableICache(void);
/**
  \fn    void DisableICache(void)
  \brief Disables Instrution cache
  \return void
 */
void DisableICache(void);
/**
  \fn    bool IsICacheEnabled(void)
  \brief Check whether cache is enabled
  \return enabled or not
 */
bool IsICacheEnabled(void);

/**
  \fn    void EnableFCache(void)
  \brief Enables Instrution cache
  \return void
 */
void EnableFCache(void);
/**
  \fn    void DisableFCache(void)
  \brief Disables Instrution cache
  \return void
 */
void DisableFCache(void);
/**
  \fn    bool IsFCacheEnabled(void)
  \brief Check whether cache is enabled
  \return enabled or not
 */
bool IsFCacheEnabled(void);


    /** \}*/

#ifdef __cplusplus
}
#endif

#endif /* _CACHE_EC7XX_H */

