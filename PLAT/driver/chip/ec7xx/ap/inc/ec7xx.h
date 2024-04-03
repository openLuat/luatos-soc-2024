
#ifndef EC_7XX_H
#define EC_7XX_H

#if defined TYPE_EC718S || defined TYPE_EC718P || defined TYPE_EC718H || defined TYPE_EC718U
#include "ec718/ec718.h"
#elif defined TYPE_EC716S || defined TYPE_EC716E
#include "ec716/ec716.h"
#else
#error "Need define correct CHIP TYPE"
#endif

#endif

