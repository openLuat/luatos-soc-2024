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
    -- little_flash
    add_includedirs(luatos_root.."/components/little_flash/inc",
                luatos_root.."/components/little_flash/port",
                {public = true})
    add_files(luatos_root.."/components/little_flash/**.c")

    add_includedirs("./inc",{public = true})
    add_files("./src/*.c",{public = true})

end)