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
    -- common
    add_includedirs(luatos_root.."/components/common",{public = true})
   
    -- audio
	add_files(luatos_root.."/components/multimedia/luat_audio_es8311.c")
	add_files(luatos_root.."/components/multimedia/luat_audio_tm8211.c")
	-- mqtt
    add_includedirs(luatos_root.."/components/network/libemqtt", {public = true})
    add_files(luatos_root.."/components/network/libemqtt/*.c")
    remove_files(luatos_root.."/components/network/libemqtt/luat_lib_mqtt.c")

end)