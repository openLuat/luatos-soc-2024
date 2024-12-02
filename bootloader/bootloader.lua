

add_defines("LTO_FEATURE_MODE",
            "DHCPD_ENABLE_DEFINE=0",
            "FEATURE_FOTA_ENABLE",
            "MIDDLEWARE_FOTA_ENABLE",
            "FEATURE_FOTA_CORE2_ENABLE",
            "USBC_USBMST_MGR_FEATURE_DISABLE=1",
            "USB_DRV_SMALL_IMAGE=1",
            "FEATURE_BOOTLOADER_PROJECT_ENABLE",
            "__BL_MODE__",
            "FEATURE_FOTA_HLS_ENABLE",
            "FEATURE_FOTA_USBURC_ENABLE",
            "FOTA_PRESET_RAM_ENABLE=1",
            "DEBUG_LOG_HEADER_FILE=\"debug_log_dummy.h\""
            )
add_cxflags("-flto",
            "-fuse-linker-plugin",
            "-ffat-lto-objects",
            "-flto-partition=none",
            "-Wno-lto-type-mismatch",
            {force=true})

target("driver",function()
    set_kind("static")
    set_targetdir(project_dir.."/build/bootloader_libdriver")
    add_includedirs(csdk_root.."/PLAT/prebuild/PLAT/inc/usb_bl")

    add_includedirs(csdk_root.."/PLAT/middleware/developed/debug/inc",
                    csdk_root.."/PLAT/project/ec7xx_0h00/ap/apps/bootloader/code/include",
                    csdk_root.."/PLAT/project/ec7xx_0h00/ap/apps/bootloader/code/include/common",
                    csdk_root.."/PLAT/project/ec7xx_0h00/ap/apps/bootloader/code/common/secure/hash/inc")
    -- wrapper
    add_includedirs(csdk_root.."/PLAT/middleware/thirdparty/lzma2201/C",
                    csdk_root.."/PLAT/middleware/thirdparty/lzma2201/C/wrapper")

	add_files(
                csdk_root.."/PLAT/core/code/boot_code.c",
                csdk_root.."/PLAT/core/code/fota_code.c",
                -- driver
                csdk_root.."/PLAT/driver/board/ec7xx_0h00/src/plat_config.c",
                csdk_root.."/PLAT/driver/hal/ec7xx/ap/src/hal_pwrkey.c",
                csdk_root.."/PLAT/driver/hal/ec7xx/ap/src/hal_misc.c",
                csdk_root.."/PLAT/driver/hal/common/src/ec_string.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/clock.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/gpio.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/oneWire.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/pad.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/wdt.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/usb_bl/open/*.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/usb_bl/usb_device/*.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/common/gcc/memcpy-armv7m.S",
				--user code
				csdk_root.."/bootloader/*.c")

	remove_files(
                csdk_root.."/PLAT/project/ec7xx_0h00/ap/apps/bootloader/code/main/system_ec7xx.c",
                csdk_root.."/PLAT/project/ec7xx_0h00/ap/apps/bootloader/code/common/image/image.c",
                csdk_root.."/PLAT/project/ec7xx_0h00/ap/apps/bootloader/code/common/secure/ecc/src/*.c"
                )
end)

target("ap_bootloader.elf",function()
    set_kind("binary")
    set_targetdir(project_dir.."/build/ap_bootloader")
    add_deps("driver")
    
    if chip_target and lib_ps_plat then

        if chip_target=="ec718u" and lib_ps_plat=="ims" then
            add_linkdirs(csdk_root.."/PLAT/libs/ec718u-ims/bootloader")
        elseif chip_target=="ec718um" and lib_ps_plat=="ims" then
            add_linkdirs(csdk_root.."/PLAT/libs/ec718um-ims/bootloader")
        else
            add_linkdirs(csdk_root.."/PLAT/libs/"..(chip_target=="ec718e"and"ec718p"or chip_target)..(lib_ps_plat=="mid"and"-mid"or"").."/bootloader")
        end
        add_linkdirs(csdk_root.."/PLAT/prebuild/PLAT/lib/gcc/"..((chip_target == "ec718um" and "ec718um") or (chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6)).."/"..lib_ps_plat)

    end
    
    add_linkdirs(csdk_root.."/lib/")
    add_linkgroups("driver","startup","core_airm2m","lzma","driver_private_bl","bootloader","usbbl_priv",
                    "osa","middleware_ec","middleware_ec_private","ccio","fota","deltapatch2","ffota_eflash", 
                    {whole = true},{group = true})

    if chip_target then
        if chip_target=="ec718um" then
            add_ldflags("-T"..csdk_root.."/PLAT/core/ld/ec718xm/ec7xx_0h00_flash_bl.ld","-Wl,-Map,"..project_dir.."/build/ap_bootloader/ap_bootloader_debug.map",{force = true})
        else
            add_ldflags("-T"..csdk_root.."/PLAT/core/ld/ec7xx_0h00_flash_bl.ld","-Wl,-Map,"..project_dir.."/build/ap_bootloader/ap_bootloader_debug.map",{force = true})
        end
    end


    local toolchains = nil
    local ld_parameter = nil 
    before_link(function(target)
        local chip_target = nil
        if has_config("chip_target") then chip_target = get_config("chip_target") end
        
        local project_dir = target:values("project_dir")
        local csdk_root = target:values("csdk_root")
        toolchains = target:tool("cc"):match('.+\\bin') or target:tool("cc"):match('.+/bin')
        for _, dep in ipairs(target:orderdeps()) do
            local linkdir = dep:targetdir()
            target:add("ldflags","-L"..csdk_root.."/"..linkdir, {force=true})
        end  
        ld_parameter = {"-E","-P"}

        local user_mem_map = {}
        for _, define_flasg in pairs(target:get("defines")) do
            table.insert(ld_parameter,"-D" .. define_flasg)
            if define_flasg == "__USER_MAP_CONF_FILE__=\"mem_map_7xx.h\"" then
                for _, filepath in ipairs(os.files(project_dir.."/**/mem_map_7xx.h")) do
                    if path.filename(filepath) == "mem_map_7xx.h" then
                        user_mem_map = {"-I",path.directory(filepath)}
                        break
                    end
                end
            end
        end

        if chip_target then
            if chip_target=="ec718um" then
                os.execv(toolchains .. "/arm-none-eabi-gcc",
                table.join(ld_parameter,user_mem_map, 
                            {"-I",csdk_root .. "/PLAT/device/target/board/ec7xx_0h00/common/pkginc"},
                            {"-I",csdk_root .. "/PLAT/device/target/board/ec7xx_0h00/common/inc"},
                            {"-I",csdk_root .. "/PLAT/core/ld"},
                            {"-o",csdk_root .. "/PLAT/core/ld/ec718xm/ec7xx_0h00_flash_bl.ld","-"}),
                            {stdin = csdk_root .. "/PLAT/core/ld/ec718xm/ec7xx_0h00_flash_bl.c"})
            else
                os.execv(toolchains .. "/arm-none-eabi-gcc",
                table.join(ld_parameter,user_mem_map, 
                            {"-I",csdk_root .. "/PLAT/device/target/board/ec7xx_0h00/common/pkginc"},
                            {"-I",csdk_root .. "/PLAT/device/target/board/ec7xx_0h00/common/inc"},
                            {"-I",csdk_root .. "/PLAT/core/ld"},
                            {"-o",csdk_root .. "/PLAT/core/ld/ec7xx_0h00_flash_bl.ld","-"}),
                            {stdin = csdk_root .. "/PLAT/core/ld/ec7xx_0h00_flash_bl.c"})
            end
        end

    end)
    after_build(function(target)
        local project_dir = target:values("project_dir")
        local csdk_root = target:values("csdk_root")
        local mem_parameter = {}
        for _, cx_flasg in pairs(target:get("cxflags")) do
            table.insert(mem_parameter,cx_flasg)
        end
        table.join2(mem_parameter,ld_parameter)
        for _, includedirs_flasg in pairs(target:get("includedirs")) do
            table.insert(mem_parameter,"-I" .. includedirs_flasg)
        end
        os.execv(toolchains .. "/arm-none-eabi-gcc",table.join(mem_parameter, {"-o",project_dir .. "/build/ap_bootloader/mem_map.txt","-"}),{stdin = csdk_root .. "/PLAT/device/target/board/ec7xx_0h00/common/inc/mem_map.h"})
        os.exec(toolchains .. "/arm-none-eabi-objcopy -O binary "..project_dir.."/build/ap_bootloader/ap_bootloader.elf "..project_dir.."/build/ap_bootloader/ap_bootloader.bin")
        os.iorun(toolchains .. "/arm-none-eabi-size "..project_dir.."/build/ap_bootloader/ap_bootloader.elf")
        os.cp(project_dir.."/build/ap_bootloader/ap_bootloader.bin", project_dir.."/build/ap_bootloader/ap_bootloader_unZip.bin")
        io.writefile(project_dir.."/build/ap_bootloader/ap_bootloader.size", os.iorun(toolchains .. "/arm-none-eabi-objdump -h "..project_dir.."/build/ap_bootloader/ap_bootloader.elf"))
        local size_file = io.open(project_dir.."/build/ap_bootloader/ap_bootloader.size", "a")
        size_file:write(os.iorun(toolchains .. "/arm-none-eabi-size -G "..project_dir.."/build/ap_bootloader/ap_bootloader.elf"))
        if ((chip_target == "ec718um" and "ec718um") or (chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6)) == "ec718p" or 
            ((chip_target == "ec718um" and "ec718um") or (chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6)) == "ec718u" or 
            ((chip_target == "ec718um" and "ec718um") or (chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6)) == "ec718um" then 
            size_file:write(os.iorun(toolchains .. "/arm-none-eabi-size -t -G "..csdk_root.."/lib/libffota_eflash.a")) 
        end
        size_file:write(os.iorun(toolchains .. "/arm-none-eabi-size -t -G "..project_dir.."/build/bootloader_libdriver/libdriver.a"))
        for _, filepath in ipairs(os.files(csdk_root.."/PLAT/libs/"..(chip_target=="ec718e"and"ec718p"or chip_target).."/bootloader/*.a")) do
            size_file:write(os.iorun(toolchains .. "/arm-none-eabi-size -t -G " .. filepath))
        end
        size_file:close()
        os.exec(csdk_root .. (is_plat("windows") and "/PLAT/tools/fcelf.exe " or "/PLAT/tools/fcelf ")..
                "-C -bin "..project_dir.."/build/ap_bootloader/ap_bootloader_unZip.bin".. 
                " -cfg ".. csdk_root.."/PLAT/project/ec7xx_0h00/ap/apps/bootloader/GCC/sectionInfo_"..((chip_target == "ec718um" and "ec718um") or (chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6))..".json".. 
                " -map "..project_dir.."/build/ap_bootloader/ap_bootloader_debug.map".." -out "..project_dir.."/build/ap_bootloader/ap_bootloader.bin")
    end)
end)






