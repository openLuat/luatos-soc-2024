
#include "mem_map.h"
#include "bl_link_mem_map.h"

/* Entry Point */
ENTRY(Reset_Handler)

/* Specify the memory areas */
MEMORY
{
  ASMB_AREA_REMAP(rwx)          : ORIGIN = ASMB_START_ADDR,            LENGTH = ASMB_TOTAL_LENGTH                   /* 64KB */
  MSMB_AREA(rwx)                : ORIGIN = MSMB_START_ADDR,            LENGTH = MSMB_TOTAL_LENGTH                   /* 160KB */
  FLASH_AREA(rx)                : ORIGIN = BOOTLOADER_FLASH_LOAD_ADDR, LENGTH = BOOTLOADER_FLASH_LOAD_UNZIP_SIZE    /* 120KB */
}

/* Define output sections */
SECTIONS
{
  .vector BOOTLOADER_FLASH_LOAD_ADDR:
  {
    KEEP(*(.isr_vector))
  } >FLASH_AREA

  .unload_uncompress_flashcode :
  {
    *startup_ec7xx_gcc.o(.text* .rodata*)
    *(.sect_bl_uncompress_flash_text.*)
    *(.sect_bl_uncompress_flash_rodata.*)
    *(.sect_cache_text.*)
    *(.sect_cache_rodata.*)
    *(.text.memset)
  } >FLASH_AREA

  .load_airam_pre1 ASMB_START_ADDR :
  {
    . = ALIGN(4);
    Load$$LOAD_AIRAM_PRE1$$Base = LOADADDR(.load_airam_pre1);
    Image$$LOAD_AIRAM_PRE1$$Base = .;
    *(.sect_bl_airam_pre1_text.*)
    *(.sect_bl_airam_pre1_rodata.*)
    . = ALIGN(4);
  } >ASMB_AREA_REMAP AT>FLASH_AREA
  Image$$LOAD_AIRAM_PRE1$$Length = SIZEOF(.load_airam_pre1);

  .load_airam_pre2 :
  {
    . = ALIGN(4);
    Load$$LOAD_AIRAM_PRE2$$Base = LOADADDR(.load_airam_pre2);
    Image$$LOAD_AIRAM_PRE2$$Base = .;
    *(.sect_bl_airam_pre2_text.*)
    *(.sect_bl_airam_pre2_rodata.*)
    *(.sect_qspi_text.*)
    *(.sect_flash_pre2text.*)
    *(.sect_platdelay_text.*)
    *(.sect_flashlock_text.*)
    . = ALIGN(4);
  } >ASMB_AREA_REMAP AT>FLASH_AREA
  Image$$LOAD_AIRAM_PRE2$$Length = SIZEOF(.load_airam_pre2);

  .unload_asmb_zi (NOLOAD):
  {
    . = ALIGN(4);
    Image$$UNLOAD_BOOT_ASMB$$ZI$$Base = .;
    *(.sect_boot_bss.*)
    . = ALIGN(4);
    Image$$UNLOAD_BOOT_ASMB$$ZI$$Limit = .;
  } >ASMB_AREA_REMAP

  PROVIDE(flashXIPLimit =LOADADDR(.load_airam_other));

  .load_airam_other :
  {
    . = ALIGN(4);
    Load$$LOAD_AIRAM_OTHER_RAMCODE$$Base = LOADADDR(.load_airam_other);
    Image$$LOAD_AIRAM_OTHER_RAMCODE$$Base = .;
    *(.rodata*)
    *(.text*)
    . = ALIGN(4);
  } >ASMB_AREA_REMAP AT>FLASH_AREA
  Image$$LOAD_AIRAM_OTHER_RAMCODE$$Length = SIZEOF(.load_airam_other);

  .load_msmb_sct_zi (NOLOAD):
  {
    . = ALIGN(4);
    Load$$LOAD_MSMB_SCT_DATA$$ZI$$Base = LOADADDR(.load_msmb_sct_zi);
    Image$$LOAD_MSMB_SCT_DATA$$ZI$$Base = .;
    *(.sect_platBlSctZIData_bss.*)
    *(.sect_decompress_bss.*)
    . = ALIGN(4);
    Image$$LOAD_MSMB_SCT_DATA$$ZI$$Limit = .;
  } >MSMB_AREA
  Image$$LOAD_MSMB_SCT_DATA$$ZI$$Length = SIZEOF(.load_msmb_sct_zi);

  .load_asmb_shared_data :
  {
    . = ALIGN(4);
    Load$$LOAD_ASMB_SHARED_DATA$$Base = LOADADDR(.load_asmb_shared_data);
    Image$$LOAD_ASMB_SHARED_DATA$$Base = .;
    *(.sect_*_data.*)
    *(.data*)
    . = ALIGN(4);
  } >ASMB_AREA_REMAP AT>FLASH_AREA
  Image$$LOAD_ASMB_SHARED_DATA$$Length = SIZEOF(.load_asmb_shared_data);

  .load_asmb_shared_zi (NOLOAD):
  {
    Image$$LOAD_ASMB_SHARED$$ZI$$Base = .;
    // add asmb zi/bss here
    Image$$LOAD_ASMB_SHARED$$ZI$$Limit = .;
    // no init data(data init by sw function init)
    *(.USB_NOINIT_DATA_BUF*)
  } >ASMB_AREA_REMAP

  .load_msmb_shared_zi (NOLOAD):
  {
    . = ALIGN(4);
    Image$$LOAD_MSMB_SHARED$$ZI$$Base = .;
    *(.sect_*_bss.*)
    *(.bss*)
    *(.stack)
    . = ALIGN(4);
    Image$$LOAD_MSMB_SHARED$$ZI$$Limit = .;
    *(.sect_boot_noInit.*)
    PROVIDE(end_msmb_software_loc = . );
  } >MSMB_AREA

  .load_ciram :
  {
    . = ALIGN(4);
    Load$$LOAD_CIRAM_RAMCODE$$Base = LOADADDR(.load_ciram);
    Image$$LOAD_CIRAM_RAMCODE$$Base = .;
    *fota_nvm.o(.text*)
    *(.sect_bl_ciram_flash_text.*)
    *(.sect_bl_ciram_flash_rodata.*)
    *(.sect_flash_text.*)
    *(.sect_platPARamcode_text.*)
    *(.memcpy.armv7m*)
    *(.glue_7)
    *(.glue_7t)
    *(.vfpll_veneer)
    *(.v4_bx)
    *(.init*)
    *(.fini*)
    *(.iplt)
    *(.igot.plt)
    *(.rel.iplt)
    . = ALIGN(4);
  } >MSMB_AREA AT>FLASH_AREA


  Image$$LOAD_CIRAM_RAMCODE$$Length = SIZEOF(.load_ciram);

  _fota_mux_buf_start = PSRAM_FOTA_MUXMEM_BASE_ADDR;
  _fota_mux_buf_end = PSRAM_FOTA_MUXMEM_END_ADDR;

  ASSERT(_fota_mux_buf_start>=end_msmb_software_loc,"bootloader software use too much msmb, overlap with fota buf!")

  _compress_buf_start = PSRAM_COMPR_MEM_BASE_ADDR;
  _compress_buf_end = PSRAM_COMPR_MEM_END_ADDR;

  _decompress_buf_start = PSRAM_DECOMPR_MEM_BASE_ADDR;
  _decompress_buf_end = PSRAM_DECOMPR_MEM_END_ADDR;

#if HEAP_EXIST
  _heap_memory_start = APVIEW_CSMB_HEAP_START;
  _heap_memory_end = APVIEW_CSMB_HEAP_END;
#endif
}

GROUP(
    libgcc.a
    libc.a
    libm.a
 )
