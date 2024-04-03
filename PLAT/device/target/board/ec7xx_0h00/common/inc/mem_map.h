
#ifndef MEM_MAP_H
#define MEM_MAP_H

/*
718S: 2M flash only
718H/V: 4M ap flash+ 1M cp flash
718P: 4M flash+ 2M psram
718U: 8M flash+ 2M psram

*/

/* invalid base addr */
#define FLASH_FOTA_ADDR_UNDEF    0xF07ADEAD


#if defined TYPE_EC718S
    #include "mem_map_csdk_718s.h"
#elif defined TYPE_EC718H
    #include "mem_map_csdk_718h.h"
#elif defined TYPE_EC718P
    #include "mem_map_csdk_718p.h"
#elif defined TYPE_EC718U
    #include "mem_map_csdk_718u.h"
#elif defined TYPE_EC716S
    #include "mem_map_csdk_716s.h"
#elif defined TYPE_EC716E
    #include "mem_map_csdk_716e.h"
#else
    #error "Need define chip type"
#endif



#endif

