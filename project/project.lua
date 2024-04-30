
function description_csdk()
    if has_config("chip_target") then 
        chip_target = get_config("chip_target")
        use_lto = false
        if chip_target == "ec716s" or chip_target == "ec718s" then 
            use_lto = true
        elseif os.getenv("LTO_FEATURE_MODE") == "enable" then 
            use_lto = true
        end

        if chip_target == "ec718p" or chip_target == "ec718pv" then
            add_defines("PSRAM_FEATURE_ENABLE")
            --add_defines("FEATURE_EXCEPTION_FLASH_DUMP_ENABLE")
        end

        if chip_target == "ec718pv" then
            add_defines("FEATURE_IMS_ENABLE",
                        "FEATURE_IMS_CC_ENABLE",
                        "FEATURE_IMS_SMS_ENABLE",
                        "FEATURE_IMS_USE_PSRAM_ENABLE",
                        -- "FEATURE_SUPPORT_APP_PCM_MEM_POOL",
                        -- "FEATURE_AUDIO_ENABLE",
                        "FEATURE_AMR_CP_ENABLE",
                        "FEATURE_VEM_CP_ENABLE")

            add_cxflags("-fno-strict-aliasing",{force=true})
        end
        
        if use_lto then
            -- 开启 lto
            add_defines("LTO_FEATURE_MODE")
            add_cxflags("-flto",
                        "-fuse-linker-plugin",
                        "-ffat-lto-objects",
                        "-flto-partition=none",
                        "-Wno-lto-type-mismatch",
                        {force=true})
        end 

        if lib_ps_plat == "mid" then
            add_defines("DHCPD_ENABLE_DEFINE=0")
        else 
            add_defines("DHCPD_ENABLE_DEFINE=1")
        end

        add_includedirs(csdk_root.."/PLAT/tools/"..(chip_target=="ec718e"and"ec718p"or chip_target)..(lib_ps_plat=="mid"and"-mid"or""))

    end

    add_cxflags("-mslow-flash-data",{force=true})

    add_defines("LUAT_BSP_VERSION=\""..LUAT_BSP_VERSION.."\"",
                "EC_ASSERT_FLAG",
                "PM_FEATURE_ENABLE",
                "UINILOG_FEATURE_ENABLE",
                "FEATURE_OS_ENABLE",
                "FEATURE_FREERTOS_ENABLE",
                "configUSE_NEWLIB_REENTRANT=1",
                "FEATURE_YRCOMPRESS_ENABLE",
                "FEATURE_CCIO_ENABLE",
                "LWIP_CONFIG_FILE=\"lwip_config_cat.h\"",
                --"LWIP_CONFIG_FILE=\"lwip_config_ec7xx0h00.h\"",
                -- "FEATURE_MBEDTLS_ENABLE",-------------
                "LFS_NAME_MAX=63",
                "LFS_DEBUG_TRACE",
                "FEATURE_UART_HELP_DUMP_ENABLE",
                "HTTPS_WITH_CA",
                "FEATURE_HTTPC_ENABLE",
                -- "LITE_FEATURE_MODE",
                "RTE_USB_EN=1",
                -- "RTE_PPP_EN=0",
                -- "RTE_OPAQ_EN=0",-----
                "RTE_ONE_UART_AT=0",
                "RTE_TWO_UART_AT=0",
                "__USER_CODE__",
                "LUAT_USE_NETWORK",
                "LUAT_USE_LWIP",
                "__USE_SDK_LWIP__",
                "LUAT_USE_DNS",
                "__PRINT_ALIGNED_32BIT__",
                "_REENT_SMALL",
                "_REENT_GLOBAL_ATEXIT",
                "LWIP_INCLUDED_POLARSSL_MD5=1",
                "LUAT_EC7XX_CSDK",
                "LUAT_USE_STD_STRING",
                "LUAT_LOG_NO_NEWLINE",
                "FEATURE_PS_SMS_AT_ENABLE",
                "DEBUG_LOG_HEADER_FILE=\"debug_log_ap.h\"")
    add_defines("sprintf=sprintf_",
                "snprintf=snprintf_",
                "vsnprintf=vsnprintf_")

    add_ldflags("-Wl,--wrap=_malloc_r",
                "-Wl,--wrap=_free_r",
                "-Wl,--wrap=_realloc_r",
                "-Wl,--wrap=clock",
                "-Wl,--wrap=localtime",
                "-Wl,--wrap=gmtime",
                "-Wl,--wrap=time",
                {force = true})

    -- 已经生效的GCC警告信息
    add_cxflags("-Werror=maybe-uninitialized")
    add_cxflags("-Werror=unused-value")
    add_cxflags("-Werror=array-bounds")
    add_cxflags("-Werror=return-type")
    add_cxflags("-Werror=overflow")
    add_cxflags("-Werror=empty-body")
    add_cxflags("-Werror=old-style-declaration")
    -- add_cxflags("-Werror=implicit-function-declaration")
    add_cxflags("-Werror=implicit-int")

    -- 暂不考虑的GCC警告信息
    add_cxflags("-Wno-unused-parameter")
    add_cxflags("-Wno-unused-but-set-variable")
    add_cxflags("-Wno-sign-compare")
    add_cxflags("-Wno-unused-variable")
    add_cxflags("-Wno-unused-function")

    -- 待修复的GCC警告信息
    add_cxflags("-Wno-int-conversion")
    add_cxflags("-Wno-discarded-qualifiers")
    add_cxflags("-Wno-pointer-sign")
    add_cxflags("-Wno-type-limits")
    add_cxflags("-Wno-incompatible-pointer-types")
    add_cxflags("-Wno-pointer-to-int-cast")
    add_cxflags("-Wno-int-to-pointer-cast")

    -- ==============================
    -- === includes =====
    -- SDK相关头文件引用
    add_includedirs(csdk_root.."/thirdparty/littlefs",
                    csdk_root.."/thirdparty/littlefs/port")
                
    -- CSDK 宏定义
    add_defines("LUAT_USE_FS_VFS","MBEDTLS_CONFIG_FILE=\"mbedtls_ec7xx_config.h\"")
    -- CSDK相关头文件引用
    add_includedirs(luatos_root .. "/luat/include",
                    luatos_root .. "/components/common",
                    luatos_root .. "/components/mobile",
                    luatos_root .. "/components/printf",
                    luatos_root .. "/components/ethernet/common",
                    luatos_root .. "/components/mbedtls",
                    luatos_root .. "/components/mbedtls/include",
                    luatos_root .. "/components/mbedtls/include/mbedtls",
                    luatos_root .. "/components/mbedtls/include/psa",
                    luatos_root .. "/components/network/adapter",
                    luatos_root .. "/components/camera",
                    luatos_root .. "/components/wlan",
                    luatos_root .. "/components/minmea",
                    luatos_root .. "/components/sms",
                    luatos_root .. "/components/lcd",
                    luatos_root .. "/components/u8g2",
                    luatos_root .. "/components/cjson",
                    luatos_root .. "/components/multimedia",
                    luatos_root .. "/components/io_queue",
                    csdk_root.."/interface/include")
end
description_csdk()

target("csdk",function()
    set_kind("static")
    set_targetdir(project_dir.."/build/csdk")
    add_deps(project_name)
    add_includedirs(csdk_root.."/PLAT/prebuild/PLAT/inc/usb")
    
    --freertos
	add_files(csdk_root.."/PLAT/os/freertos/src/*.c",
            csdk_root.."/PLAT/os/freertos/portable/gcc/*.c",
            csdk_root.."/PLAT/os/freertos/portable/mem/tlsf/*.c",
            csdk_root.."/PLAT/os/freertos/CMSIS/**.c")

    -- yrcompress
	add_files(csdk_root.."/PLAT/middleware/developed/yrcompress/*.c")

    -- wrapper
    add_includedirs(csdk_root.."/PLAT/middleware/thirdparty/lzma2201/C",
                    csdk_root.."/PLAT/middleware/thirdparty/lzma2201/C/wrapper")
    --driver
	add_files(csdk_root.."/PLAT/core/code/*.c",
            csdk_root.."/PLAT/driver/board/ec7xx_0h00/src/*c",
            csdk_root.."/PLAT/driver/hal/**.c",
            csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/*.c",
            csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/usb/open/*.c",
			csdk_root.."/PLAT/driver/chip/ec7xx/common/src/VPU/vem_cfg_default.c",
            csdk_root.."/PLAT/driver/chip/ec7xx/common/gcc/memcpy-armv7m.S")
    if CHIP then
        add_files(csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/"..CHIP.."/adc.c")
    end
	if chip_target ~= "ec718pv" then
		remove_files(csdk_root.."/PLAT/driver/hal/ec7xx/ap/src/hal_voice_eng_mem.c")
	end
	remove_files(csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/cspi.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/swdio.c",
                csdk_root.."/PLAT/driver/chip/ec7xx/ap/src/i2s.c",
                csdk_root.."/PLAT/driver/hal/ec7xx/ap/src/hal_charge.c",
                csdk_root.."/PLAT/driver/hal/ec7xx/ap/src/hal_i2s.c",
                csdk_root.."/PLAT/driver/hal/ec7xx/ap/src/hal_i2c.c",
                csdk_root.."/PLAT/driver/hal/ec7xx/ap/src/hal_pwrkey.c")

    -- interface
    add_files(csdk_root.."/interface/src/*.c")
    -- network
    add_files(luatos_root .."/components/network/adapter/luat_network_adapter.c",
            luatos_root .."/components/ethernet/common/dns_client.c"
            )
    -- multimedia
    add_files(luatos_root.."/components/multimedia/luat_multimedia_audio.c")
    -- mbedtls
    add_files(luatos_root .."/components/mbedtls/library/*.c")
    -- crypto
    add_files(luatos_root.."/components/crypto/*.c")
    -- printf
    add_files(luatos_root.."/components/printf/*.c")
    -- weak
    add_files(luatos_root.."/luat/weak/luat_spi_device.c",
            luatos_root.."/luat/weak/luat_malloc_weak.c",
            luatos_root.."/luat/weak/luat_mem_weak.c")
    -- littlefs
    add_files(csdk_root.."/thirdparty/littlefs/**.c")
    -- vfs
    add_files(luatos_root.."/luat/vfs/luat_fs_lfs2.c",
            luatos_root.."/luat/vfs/luat_vfs.c")

    -- cjson
	if chip_target == "ec718pv" then
		add_files(luatos_root.."/components/cjson/*.c")
	end

    add_files(luatos_root.."/components/minmea/minmea.c")
	add_files(luatos_root.."/components/mobile/luat_mobile_common.c")

end)

target(project_name..".elf",function()
	set_kind("binary")
    set_targetdir(project_dir.."/build/"..project_name)
    add_deps("ap_bootloader.elf", {inherit = false})
    add_deps("csdk")
    add_deps(project_name)

    local chip_target = nil
    if has_config("chip_target") then chip_target = get_config("chip_target") end
    if chip_target and lib_ps_plat then
        add_linkdirs(csdk_root.."/PLAT/prebuild/PS/lib/gcc/"..(chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6).."/"..lib_ps_plat)
        add_linkdirs(csdk_root.."/PLAT/prebuild/PLAT/lib/gcc/"..(chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6).."/"..lib_ps_plat)
        add_linkdirs(csdk_root.."/PLAT/libs/"..(chip_target=="ec718e"and"ec718p"or chip_target)..(lib_ps_plat=="mid"and"-mid"or""))
        if chip_target=="ec718pv" then
            add_linkgroups("imsnv","ims", {whole = true})
        end
    end

    add_linkdirs(csdk_root.."/lib")
    add_linkdirs(csdk_root.."/PLAT/device/target/board/ec7xx_0h00/ap/gcc/")

    add_linkgroups("ps","psl1","psif","psnv","tcpipmgr","lwip","osa","ccio","deltapatch",
                    "middleware_ec","middleware_ec_private","driver_private","usb_private",
                    "startup","core_airm2m","lzma","fota","csdk",{whole = true,group = true})

    add_linkgroups(project_name, {whole = true})

    add_ldflags("-T"..csdk_root.."/PLAT/core/ld/ec7xx_0h00_flash.ld","-Wl,-Map,"..project_dir.."/build/"..project_name.."/"..project_name.."_$(mode).map",{force = true})
    
    local out_path = nil
    local ld_parameter = nil

    before_link(function(target)
        local project_name = target:values("project_name")
        local project_dir = target:values("project_dir")
        local csdk_root = target:values("csdk_root")

        out_path = project_dir .. "/out/"
		if not os.exists(out_path) then
			os.mkdir(project_dir .. "/out/")
			os.mkdir(out_path)
		end
        local toolchains = target:tool("cc"):match('.+\\bin') or target:tool("cc"):match('.+/bin')
        for _, dep in ipairs(target:orderdeps()) do
            local linkdir = dep:targetdir()
            target:add("ldflags","-L"..csdk_root.."/"..linkdir, {force=true})
        end  
        ld_parameter = {"-E","-P"}

        for _, dep in pairs(target:orderdeps()) do
            if dep:name() ~= "driver" then
                for _, dep_define_flasg in pairs(dep:get("defines")) do
                    if dep_define_flasg:startswith("AP_FLASH_LOAD_SIZE=") or dep_define_flasg:startswith("AP_PKGIMG_LIMIT_SIZE=") or dep_define_flasg:startswith("FULL_OTA_SAVE_ADDR=") then
                        table.insert(ld_parameter,"-D" .. dep_define_flasg)
                    end
                end
            end
        end

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
        os.execv(toolchains .. "/arm-none-eabi-gcc",table.join(ld_parameter,user_mem_map, {"-I",csdk_root .. "/PLAT/device/target/board/ec7xx_0h00/common/pkginc"},{"-I",csdk_root .. "/PLAT/device/target/board/ec7xx_0h00/common/inc"}, {"-o",csdk_root .. "/PLAT/core/ld/ec7xx_0h00_flash.ld","-"}),{stdin = csdk_root .. "/PLAT/core/ld/ec7xx_0h00_flash.c"})
    end)

	after_build(function(target)
        local project_name = target:values("project_name")
        local project_dir = target:values("project_dir")
        local csdk_root = target:values("csdk_root")
        local toolchains = target:tool("cc"):match('.+\\bin') or target:tool("cc"):match('.+/bin')
        local mem_parameter = {}
        for _, cx_flasg in pairs(target:get("cxflags")) do
            table.insert(mem_parameter,cx_flasg)
        end
        table.join2(mem_parameter,ld_parameter)
        for _, includedirs_flasg in pairs(target:get("includedirs")) do
            table.insert(mem_parameter,"-I" .. includedirs_flasg)
        end
        os.execv(toolchains .. "/arm-none-eabi-gcc",table.join(mem_parameter, {"-o",out_path .. "/mem_map.txt","-"}),{stdin = csdk_root .. "/PLAT/device/target/board/ec7xx_0h00/common/inc/mem_map.h"})
        os.cp(out_path .. "/mem_map.txt", "$(buildir)/"..project_name.."/mem_map.txt")
		
        os.exec(toolchains .. "/arm-none-eabi-objcopy -O binary $(buildir)/"..project_name.."/"..project_name..".elf $(buildir)/"..project_name.."/"..project_name..".bin")
		os.exec(toolchains .."/arm-none-eabi-size $(buildir)/"..project_name.."/"..project_name..".elf")
        os.cp("$(buildir)/"..project_name.."/"..project_name..".bin", "$(buildir)/"..project_name.."/"..project_name.."_unZip.bin")

        io.writefile("$(buildir)/"..project_name.."/"..project_name..".size", os.iorun(toolchains .. "/arm-none-eabi-objdump -h $(buildir)/"..project_name.."/"..project_name..".elf"))
        local size_file = io.open("$(buildir)/"..project_name.."/"..project_name..".size", "a")
        size_file:write(os.iorun(toolchains .. "/arm-none-eabi-size -G $(buildir)/"..project_name.."/"..project_name..".elf"))
        size_file:write(os.iorun(toolchains .. "/arm-none-eabi-size -t -G $(buildir)/csdk/libcsdk.a"))
        size_file:write(os.iorun(toolchains .. "/arm-none-eabi-size -t -G $(buildir)/"..project_name.."/lib"..project_name..".a"))
        for _, filepath in ipairs(os.files(csdk_root.."/PLAT/libs/"..(chip_target=="ec718e"and"ec718p"or chip_target).."/*.a")) do
            size_file:write(os.iorun(toolchains .. "/arm-none-eabi-size -t -G " .. filepath))
        end
        size_file:close()

        os.exec(csdk_root .. (is_plat("windows") and "/PLAT/tools/fcelf.exe " or "/PLAT/tools/fcelf ").."-C -bin ".."$(buildir)/"..project_name.."/"..project_name.."_unZip.bin".. " -cfg ".. csdk_root .. "/PLAT/device/target/board/ec7xx_0h00/ap/gcc/sectionInfo_"..(chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6)..".json".. " -map ".."$(buildir)/"..project_name.."/"..project_name.. "_debug.map".." -out ".."$(buildir)/"..project_name.."/" .. project_name .. ".bin")

        os.cp("$(buildir)/"..project_name.."/*.bin", out_path)
		os.cp("$(buildir)/"..project_name.."/*.map", out_path)
		os.cp("$(buildir)/"..project_name.."/*.elf", out_path)
		os.cp(csdk_root .. "/PLAT/tools/"..(chip_target=="ec718e"and"ec718p"or chip_target)..(target:values("lib_ps_plat")=="mid"and"-mid"or"").."/comdb.txt", out_path)
        os.cp("$(buildir)/"..project_name.."/" .. project_name .. ".bin", "$(buildir)/"..project_name.."/ap.bin")
        ---------------------------------------------------------
        -------------- 这部分尚不能跨平台 -------------------------
        local binpkg = csdk_root..(is_plat("windows") and "/PLAT/tools/fcelf.exe " or "/PLAT/tools/fcelf ")..
                        "-M -input $(buildir)/ap_bootloader/ap_bootloader.bin -addrname BL_PKGIMG_LNA -flashsize BOOTLOADER_PKGIMG_LIMIT_SIZE \
                        -input $(buildir)/"..project_name.."/ap.bin -addrname AP_PKGIMG_LNA -flashsize AP_PKGIMG_LIMIT_SIZE \
                        -input "..csdk_root.."/PLAT/prebuild/FW/lib/gcc/"..(chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6).."/"..target:values("lib_fw").."/cp-demo-flash.bin -addrname CP_PKGIMG_LNA -flashsize CP_PKGIMG_LIMIT_SIZE \
                        -pkgmode 1 \
                        -banoldtool 1 \
                        -productname "..(chip_target=="ec718e"and"ec718p"or chip_target):sub(1,6):upper().."_PRD \
                        -def "..out_path .. "/mem_map.txt \
                        -outfile " .. out_path.."/"..project_name..".binpkg"

        -- 如果所在平台没有fcelf, 可注释掉下面的行, 没有binpkg生成. 
        -- 仍可使用其他工具继续刷机
        -- print("fcelf CMD --> ", binpkg)
        os.exec(binpkg)
        ---------------------------------------------------------
        
        import("core.base.json")
        import("utils.archive")
        local options = {}
        options.compress = "best"

        if os.exists(out_path.."/pack") then
            os.rmdir(out_path.."/pack")
            os.mkdir(out_path.."/pack")
        end
        os.cp(csdk_root.."/tools/pack/", out_path)
        local info_table = json.loadfile(out_path.."/pack/info.json")
        info_table["rom"]["file"] = project_name..".binpkg"

        if project_name == 'luatos' then
            local LUAT_BSP_VERSION = ""
            local conf_data = io.readfile(csdk_root.."/project/luatos/inc/luat_conf_bsp.h")
            for _, define_flasg in pairs(target:get("defines")) do
                if define_flasg:startswith("LUAT_BSP_VERSION=")  then
                    LUAT_BSP_VERSION = define_flasg:match("LUAT_BSP_VERSION=\"(%w+)\"")
                end
            end
            local VM_64BIT = conf_data:find("\r#define LUAT_CONF_VM_64bit") or conf_data:find("\n#define LUAT_CONF_VM_64bit")
            local mem_map_data = io.readfile("$(buildir)/"..project_name.."/mem_map.txt")
            local FLASH_FOTA_REGION_START = tonumber(mem_map_data:match("#define FLASH_FOTA_REGION_START%s+%((%g+)%)"))
            local FLASH_FOTA_REGION_END = tonumber(mem_map_data:match("#define FLASH_FOTA_REGION_END%s+%((%g+)%)"))
            local FLASH_FS_REGION_START = tonumber(mem_map_data:match("#define FLASH_FS_REGION_START%s+%((%g+)%)"))
            local FLASH_FS_REGION_END = tonumber(mem_map_data:match("#define FLASH_FS_REGION_END%s+%((%g+)%)"))
            local FLASH_FOTA_REGION_LEN = FLASH_FOTA_REGION_END - FLASH_FOTA_REGION_START
            -- print("FLASH_FOTA_REGION",FLASH_FOTA_REGION_START,FLASH_FOTA_REGION_END,FLASH_FOTA_REGION_LEN)
            local LUAT_SCRIPT_SIZE = tonumber(conf_data:match("\r#define LUAT_SCRIPT_SIZE (%d+)") or conf_data:match("\n#define LUAT_SCRIPT_SIZE (%d+)"))
            local LUAT_SCRIPT_OTA_SIZE = tonumber(conf_data:match("\r#define LUAT_SCRIPT_OTA_SIZE (%d+)") or conf_data:match("\n#define LUAT_SCRIPT_OTA_SIZE (%d+)"))
            -- print(string.format("script zone %d ota %d", LUAT_SCRIPT_SIZE, LUAT_SCRIPT_OTA_SIZE))
            if chip_target == "ec718pv" and LUAT_SCRIPT_SIZE > 128 then
                LUAT_SCRIPT_SIZE = 128
                LUAT_SCRIPT_OTA_SIZE = 96
            end
            local LUA_SCRIPT_ADDR = FLASH_FOTA_REGION_START - (LUAT_SCRIPT_SIZE + LUAT_SCRIPT_OTA_SIZE) * 1024
            local LUA_SCRIPT_OTA_ADDR = FLASH_FOTA_REGION_START - LUAT_SCRIPT_OTA_SIZE * 1024
            local FLASH_FS_REGION_SIZE = FLASH_FS_REGION_END - FLASH_FS_REGION_START
            local script_addr = string.format("%X", LUA_SCRIPT_ADDR)
            local full_addr = string.format("%X", LUA_SCRIPT_OTA_ADDR)
            local fota_len = string.format("%X", FLASH_FOTA_REGION_LEN)
            local fs_addr = string.format("%X", FLASH_FS_REGION_START)
            local fs_len = string.format("%X", FLASH_FS_REGION_SIZE)
            -- print("LUA_SCRIPT_ADDR",LUA_SCRIPT_ADDR)
            -- print("LUA_SCRIPT_OTA_ADDR",LUA_SCRIPT_OTA_ADDR)
            -- print("script_addr",script_addr)
            -- print("full_addr",full_addr)
            -- print("fota_len",fota_len)
            -- print("fs_addr",fs_addr)
            -- print("fs_len",fs_len)
            if VM_64BIT then
                info_table["script"]["bitw"] = 64
            end
            if script_addr then
                info_table["download"]["script_addr"] = script_addr
                info_table["download"]["fs_addr"] = fs_addr
                info_table["rom"]["fs"]["filesystem"]["offset"] = fs_addr
                info_table["rom"]["fs"]["filesystem"]["size"] = fs_len
                info_table["rom"]["fs"]["script"]["size"] = LUAT_SCRIPT_SIZE
            end
            if full_addr then
                info_table["fota"]["full_addr"] = full_addr
                info_table["fota"]["fota_len"] = fota_len
            end
            json.savefile(out_path.."/pack/info.json", info_table)
            os.cp(out_path.."/"..project_name..".binpkg", out_path.."/pack")
            os.cp(out_path.."/"..project_name..".elf", out_path.."/pack")
            os.cp(out_path.."/"..project_name.."*.map", out_path.."/pack")
            os.cp(out_path.."/comdb.txt", out_path.."/pack")
            os.cp(out_path.."/mem_map.txt", out_path.."/pack")
            os.cp(csdk_root.."/project/luatos/inc/luat_conf_bsp.h", out_path.."/pack")
            local ret = archive.archive(out_path.."/"..project_name..".7z", out_path.."/pack/*",options)
            if not ret then
                print("pls install p7zip-full in linux/mac.")
                return
            end
            local ret = archive.archive(out_path.."/"..project_name..".7z", out_path.."/pack/*",options)
            if not ret then
                print("pls install p7zip-full in linux/mac.")
                return
            end
            os.mv(out_path.."/"..project_name..".7z", out_path.."/LuatOS-SoC_"..LUAT_BSP_VERSION.."_".. chip_target:upper() ..".soc")
            os.rm(out_path.."/pack")
        else 
            json.savefile(out_path.."/pack/info.json", info_table)
            os.cp(out_path.."/"..project_name..".binpkg", out_path.."/pack")
            os.cp(out_path.."/"..project_name..".elf", out_path.."/pack")
            os.cp(out_path.."/"..project_name.."*.map", out_path.."/pack")
            os.cp(out_path.."/comdb.txt", out_path.."/pack")
            os.cp(out_path.."/mem_map.txt", out_path.."/pack")
            local ret = archive.archive(out_path.."/"..project_name..".7z", out_path.."/pack/*",options)
            if not ret then
                print("pls install p7zip-full in linux/mac , or 7zip in windows.")
                return
            end
            os.mv(out_path.."/"..project_name..".7z", out_path.."/"..project_name.."_".. chip_target ..".soc")
            os.rm(out_path.."/pack")
        end
        -- 计算差分包大小, 需要把老的binpkg放在根目录,且命名为old.binpkg
        if os.exists(csdk_root.."/old.binpkg") then
            os.cp(csdk_root.."/PLAT/tools/fcelf.exe", csdk_root.."/tools/dtools/dep/fcelf.exe")
            os.cp(out_path.."/"..project_name..".binpkg", csdk_root.."/tools/dtools/new.binpkg")
            os.cp(csdk_root.."/old.binpkg", csdk_root.."/tools/dtools/old.binpkg")
            os.exec(csdk_root.."/tools/dtools/run.bat BINPKG delta.par old.binpkg new.binpkg")
        end
	end)

end)
