
#ifndef BL_LINK_MEM_MAP_H
#define BL_LINK_MEM_MAP_H


/* -----------ram address define, TODO: need modify according to ram lauout-------------*/

//csmb start
#define CSMB_START_ADDR                 (0x0)
#define CSMB_END_ADDR                   (0x10000)
#define CSMB_TOTAL_LENGTH               (CSMB_END_ADDR-CSMB_START_ADDR)
#define APVIEW_CSMB_START_ADDR          (0x200000)      
#define APVIEW_CSMB_HEAP_START          (0x200010)      // first two word used for fast boot, keep align
#define APVIEW_CSMB_HEAP_END            (0x204000)

//csmb end


//msmb start

/*
0x00400000          |---------------------------------|
                    |      RESERVED                   |
0x00402800          |---------------------------------|
                    |                                 |
                    |---------------------------------|
                    |      FOTA MUX MEM               |
                    |---------------------------------|
                    |                                 |
0x00500000          |---------------------------------|   <---HEAP_START_ADDR(if exist)
                    |      LOAD_XP_IPCMEM             |
0x00540000          |                                 |   <---HEAP_END_ADDR

*/
#define MSMB_START_ADDR                 (0x00400000)
#if defined CHIP_EC718
#define MSMB_END_ADDR                   (0x00540000)
#elif defined CHIP_EC716
#define MSMB_END_ADDR                   (0x00500000)
#endif
#define MSMB_TOTAL_LENGTH               (MSMB_END_ADDR-MSMB_START_ADDR)

#define MSMB_FOTA_MUXMEM_BASE_ADDR      (MSMB_START_ADDR + 0x2800)
#define MSMB_FOTA_MUXMEM_END_ADDR       (MSMB_START_ADDR + 0x100000)

#ifdef FEATURE_FOTA_HLS_ENABLE
#define MSMB_COMPR_MEM_BASE_ADDR        (MSMB_FOTA_MUXMEM_BASE_ADDR)
#define MSMB_COMPR_MEM_END_ADDR         (MSMB_FOTA_MUXMEM_BASE_ADDR + 0xBA600)

#else
#define MSMB_COMPR_MEM_BASE_ADDR        (MSMB_FOTA_MUXMEM_BASE_ADDR + 0x43200)
#define MSMB_COMPR_MEM_END_ADDR         (MSMB_FOTA_MUXMEM_END_ADDR)

#endif

#define MSMB_DECOMPR_MEM_BASE_ADDR      (MSMB_COMPR_MEM_BASE_ADDR)
#define MSMB_DECOMPR_MEM_END_ADDR       (MSMB_COMPR_MEM_BASE_ADDR + 0x3CC00)

//msmb end


//asmb start
#define ASMB_START_ADDR                 (0x00000000)
#define ASMB_END_ADDR                   (0x00010000)
#define ASMB_TOTAL_LENGTH               (ASMB_END_ADDR-ASMB_START_ADDR)

//asmb end


//heap
#define HEAP_EXIST                      (1)
#define HEAP_START_ADDR                 (APVIEW_CSMB_HEAP_START)
#define HEAP_END_ADDR                   (APVIEW_CSMB_HEAP_END)
#define HEAP_TOTAL_LENGTH               (HEAP_END_ADDR-HEAP_START_ADDR)

#endif

