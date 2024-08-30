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
	-- add_defines("LUAT_USE_RNDIS",{public = true}) -- 一般公网卡可以激活多个APN，不需要LUAT_USE_RNDIS_NAT_MODE，但是很多时候用户不确定，所以还是默认打开
	add_defines("LUAT_USE_RNDIS","LUAT_USE_RNDIS_NAT_MODE",{public = true})
    --加入代码和头文件
    add_includedirs("./inc",{public = true})
    add_files("./src/*.c",{public = true})

end)