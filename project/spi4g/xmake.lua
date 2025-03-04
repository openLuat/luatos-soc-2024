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
    add_includedirs("./",{public = true})
    add_files("./*.c")

    -- mbedtls
    add_defines("LUAT_USE_TLS",{public = true})

    -- ulwip
    add_includedirs(luatos_root.."/components/network/ulwip/include",{public = true})
    add_includedirs(luatos_root.."/components/network/adapter_lwip2",{public = true})

    -- netdrv
    add_defines("LUAT_USE_NETDRV=1",{public = true})
    add_includedirs(luatos_root.."/components/network/netdrv/include",{public = true})
    add_files(luatos_root.."/components/network/netdrv/src/luat_netdrv_napt*.c")
    add_files(luatos_root.."/components/network/netdrv/src/luat_netdrv.c")

    add_defines("LUAT_NET_IP_INTERCEPT=1",{public = true})
    add_ldflags("-Wl,--wrap=ps_ip_input",{force = true})
    add_files("../luatos/src/wrap_ip_input.c")
    add_files("../luatos/src/luat_netdrv_ec7xx.c")

    add_cxflags("-Wno-address-of-packed-member",{force = true})
end)
