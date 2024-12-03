project_dir = os.scriptdir()
project_name = project_dir:match(".+[/\\]([%w_]+)")

csdk_root = "../../" --csdk根目录,可自行修改
includes(csdk_root.."csdk.lua")
description_common()

target(project_name,function()
    set_kind("static")
    set_targetdir("$(buildir)/".. project_name .. "/")
    description_csdk()
    set_warnings("error")

    local chip_target = nil
    if has_config("chip_target") then chip_target = get_config("chip_target") end
    -- 程序区缩小到1b7000，剩余2c2000 - 1b7000 = 10b000
    if chip_target == "ec718p" or chip_target == "ec718pm" or chip_target == "ec716e" or chip_target == "ec718e" then
        add_defines("AP_FLASH_LOAD_SIZE=0x1b7000",{public = true})
        add_defines("AP_PKGIMG_LIMIT_SIZE=0x1b7000",{public = true})
        add_defines("FULL_OTA_SAVE_ADDR=0x235000",{public = true})
    end
	if chip_target == "ec718u" or chip_target == "ec718um" or chip_target == "ec718hm" then
        add_defines("AP_FLASH_LOAD_SIZE=0x40b000",{public = true})
        add_defines("AP_PKGIMG_LIMIT_SIZE=0x40b000",{public = true})
        add_defines("FULL_OTA_SAVE_ADDR=0x49C000",{public = true})
    end
    --加入代码和头文件
    add_includedirs("./inc",{public = true})
    add_files("./src/*.c",{public = true})
	--ymodem
    add_includedirs(luatos_root .. "/components/ymodem")
    add_files(luatos_root .. "/components/ymodem/luat_ymodem.c")
    --http
    add_includedirs(luatos_root.."/components/network/libhttp", {public = true})
    add_includedirs(luatos_root.."/components/network/http_parser", {public = true})
    add_files(luatos_root.."/components/network/libhttp/*.c")
    remove_files(luatos_root.."/components/network/libhttp/luat_lib_http.c")
    add_files(luatos_root.."/components/network/http_parser/*.c")


	-- 这里加入tts等音频代码，只是让8Mflash的版本AP区尽可能占用空间
    -- common
    add_includedirs(luatos_root.."/components/common",{public = true})
    --加入amr编解码库
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_common/dec/include",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_nb/common/include",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_nb/dec/include",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_wb/dec/include",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/opencore-amrnb",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/opencore-amrwb",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/oscl",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_nb/enc/src",{public = true})
    -- **.c会递归所有子文件夹下的文件
    add_files(luatos_root .. "/components/multimedia/amr_decode/**.c",{public = true})
	add_files(luatos_root.."/components/multimedia/luat_audio_es8311.c")
	add_files(luatos_root.."/components/multimedia/luat_audio_tm8211.c")
    add_linkdirs(csdk_root.."/lib",csdk_root.."/PLAT/core/lib",{public = true})
    -- -- 此处使用 libaisound50_16K.a, 还可选择 8K版本:libaisound50_8K.a,8K英文版本:libaisound50_8K_eng.a,16K英文版本:libaisound50_16K_eng.a
    add_linkgroups("mp3","tts_res","aisound50_16K", {whole = true,public = true})
end)