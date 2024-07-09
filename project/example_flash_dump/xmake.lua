project_dir = os.scriptdir()
project_name = project_dir:match(".+[/\\]([%w_]+)")

csdk_root = "../../" --csdk根目录,可自行修改
includes(csdk_root.."csdk.lua")
description_common()
--add_undefines("LUAT_USE_NETWORK")
--add_undefines("LUAT_USE_LWIP")
--add_undefines("__USE_SDK_LWIP__")
--add_undefines("LUAT_USE_DNS")
target(project_name,function()
    set_kind("static")
    set_targetdir("$(buildir)/".. project_name .. "/")
    description_csdk()
    set_warnings("error")

    --加入代码和头文件
    add_includedirs("./inc",{public = true})
    add_files("./src/*.c",{public = true})
end)
