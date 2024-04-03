local TARGET_NAME = os.scriptdir():match(".+[/\\]([%w_]+)")
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

    add_defines("LUAT_USE_DHCP",{public = true})
    add_defines("LUAT_USE_W5500",{public = true})
    add_defines("LUAT_USE_MOBILE",{public = true})
    -- LUAT_USE_W5500

    add_includedirs(luatos_root.."/components/ethernet/common", {public = true})
    add_files(luatos_root.."/components/ethernet/common/*.c")
    remove_files(luatos_root.."/components/ethernet/common/luat_network_adapter.c")
    remove_files(luatos_root.."/components/ethernet/common/dns_client.c")

    add_includedirs(luatos_root.."/components/ethernet/w5500", {public = true})
    add_files(luatos_root.."/components/ethernet/w5500/*.c")
    remove_files(luatos_root.."/components/ethernet/w5500/luat_lib_w5500.c")
    
    --加入代码和头文件
    add_includedirs("./inc",{public = true})
    add_files("./src/*.c",{public = true})
end)