project_dir = os.scriptdir()
project_name = project_dir:match(".+[/\\]([%w_]+)")

csdk_root = "../../" --csdk根目录,可自行修改
includes(csdk_root.."csdk.lua")
description_common()

target(project_name,function()
    set_kind("static")
    set_targetdir("$(buildir)/".. project_name .. "/")
    description_csdk()
    -- set_warnings("error")

    --加入代码和头文件
    add_includedirs("./inc",{public = true})
    add_files("./src/*.c",{public = true})

	--lcd
	add_includedirs(luatos_root.."/components/lcd", {public = true})
    add_files(luatos_root.."/components/lcd/*.c")
	remove_files(luatos_root.."/components/lcd/luat_lib_lcd.c")
	--jpeg
	add_includedirs(luatos_root.."/components/tiny_jpeg", {public = true})
    add_files(luatos_root.."/components/tiny_jpeg/*.c")

    add_linkdirs(csdk_root.."/lib",csdk_root.."/PLAT/core/lib",{public = true})
	add_linkgroups("image_decoder_0", "mm_common","mm_jpeg","mm_videoutil",{whole = true,public = true})

end)