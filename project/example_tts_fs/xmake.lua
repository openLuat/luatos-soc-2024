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

    --加入代码和头文件
    add_includedirs("./inc",{public = true})
    add_files("./src/*.c",{public = true})
    add_defines("LUAT_USE_SFUD",{public = true})

    -- --sfud
    -- add_includedirs(luatos_root.."/components/sfud",{public = true})
    -- add_files(luatos_root.."/components/sfud/*.c")
    -- remove_files(luatos_root.."/components/sfud/luat_lib_sfud.c")

    -- little_flash
    add_defines("LUAT_USE_LITTLE_FLASH",{public = true})
    add_includedirs(luatos_root.."/components/little_flash/inc",
                luatos_root.."/components/little_flash/port",
                {public = true})
    add_files(luatos_root.."/components/little_flash/**.c")
    remove_files(luatos_root.."/components/little_flash/luat_lib_little_flash.c")

	--codec
	add_files(luatos_root.."/components/multimedia/luat_audio_es8311.c")
	add_files(luatos_root.."/components/multimedia/luat_audio_tm8211.c")
	
    add_linkdirs(csdk_root.."/lib",csdk_root.."/PLAT/core/lib",{public = true})
    -- 此处使用 libaisound50_16K.a, 还可选择 8K版本:libaisound50_8K.a,8K英文版本:libaisound50_8K_eng.a,16K英文版本:libaisound50_16K_eng.a
    add_linkgroups("tts_res", "aisound50_16K", {whole = true,public = true})
-- 
end)