
#ifndef HAL_POW_BACKOFF_H
#define HAL_POW_BACKOFF_H

/**
  \fn          void powBackOffEnable(void)
  \brief       Enable power backoff when temperature too high. The threshold can change in hal_powBackOff.c
  \return      null
*/
void powBackOffEnable(void);
/**
  \fn          void powBackOffDisable(void)
  \brief       Disable power backoff
  \return      null
*/
void powBackOffDisable(void);


#endif

