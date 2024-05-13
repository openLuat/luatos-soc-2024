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
    add_includedirs("./include",{public = true})
    add_files("./src/*.c",{public = true})
    -- common
    add_includedirs(luatos_root.."/components/common",{public = true})
    -- **.c会递归所有子文件夹下的文件
	add_files(luatos_root.."/components/multimedia/luat_audio_es8311.c")
	add_files(luatos_root.."/components/multimedia/luat_audio_tm8211.c")
	
	-- mbedtls
    add_defines("LUAT_USE_TLS",{public = true})

	-- http
    add_includedirs(luatos_root.."/components/network/libhttp", {public = true})
    add_includedirs(luatos_root.."/components/network/http_parser", {public = true})
    add_files(luatos_root.."/components/network/libhttp/*.c")
    remove_files(luatos_root.."/components/network/libhttp/luat_lib_http.c")
    add_files(luatos_root.."/components/network/http_parser/*.c")
	
    add_linkdirs(csdk_root.."/lib",csdk_root.."/PLAT/core/lib",{public = true})
    -- 加入MP3解码库
    add_linkgroups("mp3", {whole = true,public = true})

end)