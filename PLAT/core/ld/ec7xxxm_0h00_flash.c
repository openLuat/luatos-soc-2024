
#include "mem_map.h"

/* Entry Point */
ENTRY(Reset_Handler)

/* Specify the memory areas */
MEMORY
{
    ASMB_AREA(rwx)              : ORIGIN = ASMB_START_ADDR,    LENGTH = ASMB_TOTAL_LENGTH          /* 64KB */
    MSMB_AREA(rwx)              : ORIGIN = MSMB_START_ADDR,    LENGTH = MSMB_TOTAL_LENGTH          /* 128KB */
    FLASH_AREA(rx)              : ORIGIN = AP_FLASH_LOAD_ADDR, LENGTH = AP_FLASH_LOAD_UNZIP_SIZE   /* 2.5MB */
    SHARED_MEM_AREA(rwx)        : ORIGIN = SHARED_MEM_START_ADDR|PSRAM_PCACHE0_BASE,  LENGTH =  SHARED_MEM_LENGTH     /* 8KB */
    PSRAM_P0_AREA(rwx)          : ORIGIN = PSRAM_P0_START_ADDR|PSRAM_PCACHE0_BASE,   LENGTH = PSRAM_P0_LENGTH         /* x KB */
    PSRAM_P1_AREA(rwx)          : ORIGIN = PSRAM_P1_START_ADDR|PSRAM_PCACHE1_BASE,   LENGTH = PSRAM_P1_LENGTH         /* 1016 - x KB */
    PSRAM_P2_AREA(rwx)          : ORIGIN = PSRAM_P2_START_ADDR|PSRAM_PCACHE2_BASE,   LENGTH = PSRAM_P2_LENGTH         /* 2.75MB */
}


/* Define output sections */
SECTIONS
{
    . = AP_FLASH_LOAD_ADDR;
    .vector :
    {
        KEEP(*(.isr_vector))
    } >FLASH_AREA

    .text :
    {
        *(.glue_7)
        *(.glue_7t)
        *(.vfpll_veneer)
        *(.v4_bx)
        *(.init*)
        *(.fini*)
        *(.iplt)
        *(.igot.plt)
        *(.rel.iplt)
        EXCLUDE_FILE(*libc_nano.a:lib_a-memset.o) *libc_nano.a:(.text*)
        *(.sect_cache_text.*)
        KEEP(*(SORT_BY_NAME(.sect_commonly_used_text*)))
        KEEP(*(SORT_BY_NAME(.sect_commonly_used_ro*)))
        *(.*rodata*)        /* .rodata* sections (constants, strings, etc.) */
        *(EXCLUDE_FILE(*libc*.a:*memset.o) .text*)
    } >FLASH_AREA
    
    .preinit_fun_array :
    {
        . = ALIGN(4);
        __preinit_fun_array_start = .;
        KEEP (*(SORT(.preinit_fun_array.*)))
        KEEP (*(.preinit_fun_array*))
        __preinit_fun_array_end = .;
        . = ALIGN(4);
    } > FLASH_AREA
    .drv_init_fun_array :
    {
        . = ALIGN(4);
        __drv_init_fun_array_start = .;
        KEEP (*(SORT(.drv_init_fun_array.*)))
        KEEP (*(.drv_init_fun_array*))
        __drv_init_fun_array_end = .;
        . = ALIGN(4);
    } > FLASH_AREA
    .task_fun_array :
    {
        . = ALIGN(4);
        __task_fun_array_start = .;
        KEEP (*(SORT(.task_fun_array.*)))
        KEEP (*(.task_fun_array*))
        __task_fun_array_end = .;
        . = ALIGN(4);
    } > FLASH_AREA
  
    .load_ap_piram_uncomp_msmb : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_PIRAM_UNCOMP_MSMB$$Base = LOADADDR(.load_ap_piram_uncomp_msmb);
        Image$$LOAD_AP_PIRAM_UNCOMP_MSMB$$Base = .;
        *(.sect_uncompress_platPMRamcode_text.*)   
        . = ALIGN(4);
    } >MSMB_AREA AT>FLASH_AREA

    Image$$LOAD_AP_PIRAM_UNCOMP_MSMB$$Length = SIZEOF(.load_ap_piram_uncomp_msmb);

    // psram part3 for customer
    .csdk_sect_ap_psram_p2_unued  (NOLOAD): ALIGN(4)
    {
        . = ALIGN(4);
        *(.csdk_sect_ap_psram_p2_unued.*)
        . = ALIGN(4);
    } >PSRAM_P2_AREA
    
    .load_ap_psram_p2_ram : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_PSRAM_P2_RAM$$Base = LOADADDR(.load_ap_psram_p2_ram);
        Image$$LOAD_AP_PSRAM_P2_RAM$$Base = .;
        *(.cust_sect_ap_psram_p2_text.*)
        . = ALIGN(4);
    } >PSRAM_P2_AREA AT>FLASH_AREA
    Image$$LOAD_AP_PSRAM_P2_RAM$$Length = SIZEOF(.load_ap_psram_p2_ram);

    .load_ap_psram_p2_data : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_PSRAM_P2_DATA$$Base = LOADADDR(.load_ap_psram_p2_data);
        Image$$LOAD_AP_PSRAM_P2_DATA$$Base = .;
        *(.cust_sect_ap_psram_p2_data.*)
        *(EXCLUDE_FILE(*libc_nano.a *libc.a) .data*)
        . = ALIGN(4);
    } >PSRAM_P2_AREA AT>FLASH_AREA
    Image$$LOAD_AP_PSRAM_P2_DATA$$Length = SIZEOF(.load_ap_psram_p2_data);

    .load_ap_psram_p2_bss (NOLOAD):
    {
        . = ALIGN(4);
        Image$$LOAD_AP_PSRAM_P2_ZI$$Base = .;
        *(.cust_sect_ap_psram_p2_bss.*)
        *(EXCLUDE_FILE(*libc_nano.a *libc.a) .bss*)
        . = ALIGN(8);
        Image$$LOAD_AP_PSRAM_P2_ZI$$Limit = .;
    } >PSRAM_P2_AREA

    PROVIDE(end_ap_data_psram = . );
    PROVIDE(heap_endAddr_psram = heap_boundary_psram);
    cust_heap_size = heap_endAddr_psram - end_ap_data_psram;
    ASSERT(cust_heap_size>=cust_min_heap_size_threshold,"customer use too much psram, heap less than cust_min_heap_size_threshold!")
    ASSERT(heap_endAddr_psram<=PSRAM_END_ADDR,"customer use too much psram, exceed to PSRAM_END_ADDR")

    PROVIDE(flashXIPLimit = LOADADDR(.load_bootcode));

    .load_bootcode ASMB_START_ADDR : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_BOOTCODE$$Base = LOADADDR(.load_bootcode);
        Image$$LOAD_BOOTCODE$$Base = .;
        KEEP(*(.mcuVector))
        *(.sect_ramBootCode_text.*)
        *(.sect_qspi_text.*)
        *(.sect_flash_text.*)
        *(.sect_flash_pre2text.*)
        *(.sect_flashlock_text.*)
        . = ALIGN(4);
    } >ASMB_AREA AT>FLASH_AREA

    Image$$LOAD_BOOTCODE$$Length = SIZEOF(.load_bootcode);

    .load_ap_piram_asmb : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_PIRAM_ASMB$$Base = LOADADDR(.load_ap_piram_asmb);
        Image$$LOAD_AP_PIRAM_ASMB$$Base = .;
        *(.sect_psPARamcode_text.*)
        *(.sect_platPARamcode_text.*)
        *(.sect_platdelay_text.*)
        *(.text.memset)
        *(.memcpy.armv7m)
        *(.recordNodeRO)
        . = ALIGN(4);
    } >ASMB_AREA AT>FLASH_AREA

    Image$$LOAD_AP_PIRAM_ASMB$$Length = SIZEOF(.load_ap_piram_asmb);

    .load_ap_firam_asmb : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_FIRAM_ASMB$$Base = LOADADDR(.load_ap_firam_asmb);
        Image$$LOAD_AP_FIRAM_ASMB$$Base = .;
        *(.sect_psFARamcode_text.*)
        *(.sect_platFARamcode_text.*)
        . = ALIGN(4);
    } >ASMB_AREA AT>FLASH_AREA

    Image$$LOAD_AP_FIRAM_ASMB$$Length = SIZEOF(.load_ap_firam_asmb);

    .load_ap_rwdata_asmb : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_FDATA_ASMB$$RW$$Base = LOADADDR(.load_ap_rwdata_asmb);
        Image$$LOAD_AP_FDATA_ASMB$$RW$$Base = .;
        *(.sect_platFARWData_data.*)
        . = ALIGN(4);
    } >ASMB_AREA AT>FLASH_AREA
    Image$$LOAD_AP_FDATA_ASMB$$Length = SIZEOF(.load_ap_rwdata_asmb);

    .load_ps_rwdata_asmb : ALIGN(4)
    {
        Load$$LOAD_PS_FDATA_ASMB$$RW$$Base = LOADADDR(.load_ps_rwdata_asmb);
        Image$$LOAD_PS_FDATA_ASMB$$RW$$Base = .;
        *(.sect_psFARWData_data.*)
        . = ALIGN(4);
    } >ASMB_AREA AT>FLASH_AREA
    Image$$LOAD_PS_FDATA_ASMB$$RW$$Length = SIZEOF(.load_ps_rwdata_asmb);

    .load_ap_zidata_asmb (NOLOAD):
    {
        . = ALIGN(4);
        Image$$LOAD_AP_FDATA_ASMB$$ZI$$Base = .;
        *(.sect_platFAZIData_bss.*)
        . = ALIGN(4);
        Image$$LOAD_AP_FDATA_ASMB$$ZI$$Limit = .;

        Image$$LOAD_PS_FDATA_ASMB$$ZI$$Base = .;
        *(.sect_psFAZIData_bss.*)
        . = ALIGN(4);
        Image$$LOAD_PS_FDATA_ASMB$$ZI$$Limit = .;
        *(.sect_platPANoInit_bss.*)
        *(.sect_psFANoInit_data.*)
        *(.exceptCheck)
        *(.decompress)
    } >ASMB_AREA

    asmb_flex_area = .;
    // from asmb_flex_area to CP_AONMEMBACKUP_START_ADDR is flexible for customer to store sleep2 retention data

#if defined FEATURE_FREERTOS_ENABLE
    .load_apos : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_APOS$$Base = LOADADDR(.load_apos);
        Image$$LOAD_APOS$$Base = .;
        *(.sect_freertos_eventgroups_text.*)
        *(.sect_freertos_heap6_text.*)
#if PSRAM_EXIST
        *(.sect_freertos_psram_heap6_text.*)
#endif
        *(.sect_freertos_list_text.*)
        *(.sect_freertos_queue_text.*)
        *(.sect_freertos_tasks_text.*)
        *(.sect_freertos_timers_text.*)
        *(.sect_freertos_port_text.*)
        . = ALIGN(4);
#ifndef  FEATURE_IMS_ENABLE
    } >ASMB_AREA AT>FLASH_AREA
#else
    } >MSMB_AREA AT>FLASH_AREA
#endif
    Image$$LOAD_APOS$$Length = SIZEOF(.load_apos);
#elif defined FEATURE_LITEOS_ENABLE
    .load_apos : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_APOS$$Base = LOADADDR(.load_apos);
        Image$$LOAD_APOS$$Base = .;
        *(.sect_liteos_sec_text.*)
        . = ALIGN(4);
#ifndef  FEATURE_IMS_ENABLE
    } >ASMB_AREA AT>FLASH_AREA
#else
    } >MSMB_AREA AT>FLASH_AREA
#endif
    Image$$LOAD_APOS$$Length = SIZEOF(.load_apos);
#endif

    .unload_cpaon CP_AONMEMBACKUP_START_ADDR (NOLOAD):
    {
        KEEP(*(.sect_rawflash_bss.*))		// keep this dummy section for overlay check, cp use this memory
    } >ASMB_AREA

    .load_rrcmem ASMB_RRCMEM_START_ADDR (NOLOAD):
    {
        *(.rrcMem)
    } >ASMB_AREA

    .load_flashmem ASMB_FLASHMEM_START_ADDR (NOLOAD):
    {
        *(.apFlashMem)
    } >ASMB_AREA

    .load_ap_piram_msmb : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_PIRAM_MSMB$$Base = LOADADDR(.load_ap_piram_msmb);
        Image$$LOAD_AP_PIRAM_MSMB$$Base = .;
        *(.sect_platPMRamcode_text.*)
        *(.sect_psPMRamcode_text.*)
        . = ALIGN(4);
    } >MSMB_AREA AT>FLASH_AREA

    Image$$LOAD_AP_PIRAM_MSMB$$Length = SIZEOF(.load_ap_piram_msmb);

    .load_ap_firam_msmb : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_FIRAM_MSMB$$Base = LOADADDR(.load_ap_firam_msmb);
        Image$$LOAD_AP_FIRAM_MSMB$$Base = .;
        #if defined FEATURE_FREERTOS_ENABLE
        *(.sect_freertos_cmsisos2_text.*)
        *(.sect_freertos_tlsf_text.*)
        #elif defined FEATURE_LITEOS_ENABLE
        *(.sect_liteos_cmsisos2_text.*)
        #endif
        *(.sect_platFMRamcode_text.*)
        . = ALIGN(4);
    } >MSMB_AREA AT>FLASH_AREA

    Image$$LOAD_AP_FIRAM_MSMB$$Length = SIZEOF(.load_ap_firam_msmb);

    .load_ap_ps_firam_msmb : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_PS_FIRAM_MSMB$$Base = LOADADDR(.load_ap_ps_firam_msmb);
        Image$$LOAD_AP_PS_FIRAM_MSMB$$Base = .;
        *(.sect_psFMRamcode_text.*)
        . = ALIGN(4);
    } >MSMB_AREA AT>FLASH_AREA
    Image$$LOAD_AP_PS_FIRAM_MSMB$$Length = SIZEOF(.load_ap_ps_firam_msmb);

    // plat and ps's MSMB data section
    .load_ap_msmb_data : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_MSMB_DATA$$Base = LOADADDR(.load_ap_msmb_data);
        Image$$LOAD_AP_MSMB_DATA$$Base = .;
        *(.sect_psFMRWData_data.*)
        *(.sect_platFMRWData_data.*)
        . = ALIGN(4);
    } >MSMB_AREA AT>FLASH_AREA
    Image$$LOAD_AP_MSMB_DATA$$Length = SIZEOF(.load_ap_msmb_data);

    .load_ap_boot_bss_msmb (NOLOAD):
    {
        . = ALIGN(4);
        Image$$LOAD_AP_BOOT_MSMB_BSS$$Base = .;
        *(.sect_boot_bss.*)
        . = ALIGN(4);
        Image$$LOAD_AP_BOOT_MSMB_BSS$$Limit = .;
        *(.sect_boot_noInit.*)
        *(.sleepmem)
        *(.sect_platMsmbNoInit*)
        *(.stack*)               /* stack should be 4 byte align */
        . = ALIGN(4);
    } >MSMB_AREA
	
    // plat and ps's MSMB bss section
    .load_ap_msmb_bss (NOLOAD):
    {
        . = ALIGN(4);
        Image$$LOAD_AP_MSMB_BSS$$Base = .;
        *(.sect_psFMZIData_bss.*)
        *(.sect_platFMZIData_bss.*)
        . = ALIGN(4);
        Image$$LOAD_AP_MSMB_BSS$$Limit = .;
    } >MSMB_AREA

    PROVIDE(PS_MSMB_boundary_check = . );
    ASSERT(PS_MSMB_boundary_check<=MSMB_PS_END_ADDR,"PS use too much MSMB ram!")

    // psram part1
    .load_ap_fpsram_p0_ram_plat : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_FPSRAM_P0_RAM_PLAT$$Base = LOADADDR(.load_ap_fpsram_p0_ram_plat);
        Image$$LOAD_AP_FPSRAM_P0_RAM_PLAT$$Base = .;
        *(.sect_platRamcode_psram_p0_text.*)
        . = ALIGN(4);
    } >PSRAM_P0_AREA AT>FLASH_AREA
    Image$$LOAD_AP_FPSRAM_P0_RAM_PLAT$$Length = SIZEOF(.load_ap_fpsram_p0_ram_plat);

    .load_ap_fpsram_p0_ram_ps : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_FPSRAM_P0_RAM_PS$$Base = LOADADDR(.load_ap_fpsram_p0_ram_ps);
        Image$$LOAD_AP_FPSRAM_P0_RAM_PS$$Base = .;
        *(.sect_psRamcode_psram_p0_text.*)
        . = ALIGN(4);
    } >PSRAM_P0_AREA AT>FLASH_AREA
    Image$$LOAD_AP_FPSRAM_P0_RAM_PS$$Length = SIZEOF(.load_ap_fpsram_p0_ram_ps);

    .load_ap_ppsram_p0_bsp_data : ALIGN(4)
    {
    	. = ALIGN(4);
        Load$$LOAD_PPSRAM_P0_BSP_DATA$$Base = LOADADDR(.load_ap_ppsram_p0_bsp_data);
        Image$$LOAD_PPSRAM_P0_BSP_DATA$$Base = .;
        *(.sect_bsp_spi_data.*)
        *(.sect_flash_data.*)
        *(.sect_flash_rt_data.*)
        *(.sect_gpr_data.*)
        *(.sect_apmu_data.*)
        *(.sect_apmutiming_data.*)
        *(.sect_bsp_data.*)
        *(.sect_platconfig_data.*)
        *(.sect_system_data.*)
        *(.sect_unilog_data.*)
        *(.sect_pad_data.*)
        *(.sect_ic_data.*)
        *(.sect_ecmain_data.*)
        *(.sect_slpman_data.*)
        *(.sect_bsp_usart_data.*)
        *(.sect_bsp_lpusart_data.*)
        *(.sect_bsp_can_data.*)
        *(.sect_timer_data.*)
        *(.sect_dma_data.*)
        *(.sect_adc_data.*)
        *(.sect_wdt_data.*)
        *(.sect_uart_device_data.*)
        *(.sect_usb_device_data.*)
        *(.sect_spi_device_data.*)
        *(.sect_clock_data.*)
        *(.sect_hal_adc_data.*)
        *(.sect_hal_adcproxy_data.*)
        *(.sect_hal_alarm_data.*)
        *(.sect_excep_dump_data.*)
        *(*libc*.a .data*)
        . = ALIGN(4);
    } >PSRAM_P0_AREA AT>FLASH_AREA
    Image$$LOAD_PPSRAM_P0_BSP_DATA$$Length = SIZEOF(.load_ap_ppsram_p0_bsp_data);

    .load_ap_ppsram_p0_bsp_zi (NOLOAD):
    {
        . = ALIGN(4);
        Image$$LOAD_PPSRAM_P0_BSP$$ZI$$Base = .;
        *(.sect_bsp_spi_bss.*)
        *(.sect_flash_bss.*)
        *(.sect_flash_rt_bss.*)
        *(.sect_gpr_bss.*)
        *(.sect_apmu_bss.*)
        *(.sect_apmutiming_bss.*)
        *(.sect_bsp_bss.*)
        *(.sect_platconfig_bss.*)
        *(.sect_system_bss.*)
        *(.sect_unilog_bss.*)
        *(.sect_pad_bss.*)
        *(.sect_ic_bss.*)
        *(.sect_ecmain_bss.*)
        *(.sect_slpman_bss.*)
        *(.sect_bsp_usart_bss.*)
        *(.sect_bsp_lpusart_bss.*)
        *(.sect_bsp_can_bss.*)
        *(.sect_timer_bss.*)
        *(.sect_dma_bss.*)
        *(.sect_adc_bss.*)
        *(.sect_wdt_bss.*)
        *(.sect_uart_device_bss.*)
        *(.sect_usb_device_bss.*)
        *(.sect_spi_device_bss.*)
        *(.sect_clock_bss.*)
        *(.sect_adc_bss.*)
        *(.sect_hal_trim_bss.*)
        *(.sect_hal_adcproxy_bss.*)
        *(.sect_hal_alarm_bss.*)
        *(.sect_alarm_bss.*)
        *(.sect_excep_dump_bss.*)
        *(.recordNodeZI)
        *(*libc*.a .bss*)
        . = ALIGN(4);
        Image$$LOAD_PPSRAM_P0_BSP$$ZI$$Limit = .;
    } >PSRAM_P0_AREA

    .load_ap_fpsram_p0_data : ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_FPSRAM_P0_DATA$$Base = LOADADDR(.load_ap_fpsram_p0_data);
        Image$$LOAD_AP_FPSRAM_P0_DATA$$Base = .;
        *(.sect_ap_psram_p0_data.*)
        *(.sect_ap_ps_psram_p0_data.*)
        . = ALIGN(4);
    } >PSRAM_P0_AREA AT>FLASH_AREA
    Image$$LOAD_AP_FPSRAM_P0_DATA$$Length = SIZEOF(.load_ap_fpsram_p0_data);

    .load_ap_fpsram_p0_bss (NOLOAD):
    {
        . = ALIGN(4);
        Image$$LOAD_AP_FPSRAM_P0_ZI$$Base = .;
        *(.sect_ap_psram_p0_bss.*)
        *(.sect_ap_ps_psram_p0_bss.*)
        *(.sect_*_bss*)
        *(COMMON)	
        *(.sect_decompress_bss.*)
        . = ALIGN(4);
        Image$$LOAD_AP_FPSRAM_P0_ZI$$Limit = .;

        *(.USB_NOINIT_DATA_BUF)
        *(.sect_dump_unilog.*)
    } >PSRAM_P0_AREA

    PROVIDE(ap_psram_p0_boundary = . );

    .load_ap_fpsram_p1_data .|PSRAM_PCACHE1_BASE: ALIGN(4)
    {
        . = ALIGN(4);
        Load$$LOAD_AP_FPSRAM_P1_DATA$$Base = LOADADDR(.load_ap_fpsram_p1_data);
        Image$$LOAD_AP_FPSRAM_P1_DATA$$Base = .;
        *(.sect_ap_ps_psram_p1_data.*)
        *(.sect_ap_plat_psram_p1_data.*)
        . = ALIGN(4);
    } >PSRAM_P1_AREA AT>FLASH_AREA
    Image$$LOAD_AP_FPSRAM_P1_DATA$$Length = SIZEOF(.load_ap_fpsram_p1_data);

    .load_ap_fpsram_p1_bss (.|PSRAM_PCACHE1_BASE) (NOLOAD):
    {
        . = ALIGN(4);
        Image$$LOAD_AP_FPSRAM_P1_ZI$$Base = .;
        *(.sect_ap_ps_psram_p1_bss.*)
        *(.sect_ap_plat_psram_p1_bss.*)
        . = ALIGN(4);
        Image$$LOAD_AP_FPSRAM_P1_ZI$$Limit = .;
    } >PSRAM_P1_AREA
    
	.unload_voiceEng_buffer (.|PSRAM_PCACHE1_BASE) (NOLOAD):
    {
        . = ALIGN(4);
        *(.sect_voiceEngSharebuf.*)
        . = ALIGN(4);
    } >PSRAM_P1_AREA

    .unload_up_buffer (.|PSRAM_PCACHE1_BASE) (NOLOAD):
    {
        . = ALIGN(1024);
        *(.sect_catShareBuf_data.*)
        *(.sect_ccio_buf.*)
        . = ALIGN(4);
    } >PSRAM_P1_AREA

    . = ALIGN(4);
    PROVIDE(end_ap_data = .|PSRAM_PCACHE1_BASE);
    PROVIDE(start_up_buffer = up_buf_start);
    heap_size = start_up_buffer - end_ap_data;
    asmbFlexSize = CP_AONMEMBACKUP_START_ADDR - asmb_flex_area;
    //ASSERT(heap_size>=min_heap_size_threshold,"ap use too much ram, heap less than min_heap_size_threshold!")
    #if !defined FEATURE_IMS_ENABLE
    ASSERT(asmbFlexSize>=0x1000, "we should reserve at least 4KB for user")
    #endif

    // AP&CP shared memeory sections 
    .load_xp_sharedinfo XP_SHAREINFO_BASE_ADDR (NOLOAD):
    {
        *(.shareInfo)
        Image$$SHARE_EXCEP_INFO$$ZI$$Base = .;
        *(.shareExcepInfo)
        . = ALIGN(4);
        Image$$SHARE_EXCEP_INFO$$ZI$$Limit = .;
    } >SHARED_MEM_AREA
    
    .load_dbg_area XP_DBGRESERVED_BASE_ADDR (NOLOAD):
    {
        *(.resetFlag)
    } >SHARED_MEM_AREA
    
    .unload_xp_ipcmem IPC_SHAREDMEM_START_ADDR (NOLOAD):
    {

    } >SHARED_MEM_AREA

}

GROUP(
    libgcc.a
    libc.a
    libm.a
 )
