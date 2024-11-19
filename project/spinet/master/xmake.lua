project_dir = os.scriptdir()
project_name = project_dir:match(".+[/\\]([%w_]+)")

csdk_root = "../../../" --csdk根目录,可自行修改
includes(csdk_root.."csdk.lua")
description_common()

target(project_name,function()
    set_kind("static")
    set_targetdir("$(buildir)/".. project_name .. "/")
    description_csdk()
    set_warnings("error")

    --加入代码和头文件
    add_includedirs("./",{public = true})
	add_includedirs("../common/",{public = true})
    add_files("./*.c",{public = true})
    add_files("../common/*.c",{public = true})
	
	add_includedirs(luatos_root.."/components/network/libemqtt", {public = true})
    add_files(luatos_root.."/components/network/libemqtt/*.c")
    remove_files(luatos_root.."/components/network/libemqtt/luat_lib_mqtt.c")
	-- mbedtls
    add_defines("LUAT_USE_TLS",{public = true})
end)
