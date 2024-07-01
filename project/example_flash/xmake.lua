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

    local chip_target = nil
    if has_config("chip_target") then chip_target = get_config("chip_target") end
    if chip_target == "ec718p" then
	-- 718P程序区缩小到1b7000，剩余2ca000 - 1b7000 = 113000 1100KB空间为用户区
       add_defines("AP_FLASH_LOAD_SIZE=0x1b7000",{public = true})
       add_defines("AP_PKGIMG_LIMIT_SIZE=0x1b7000",{public = true})
	   -- add_defines("FULL_OTA_SAVE_ADDR=0x235000",{public = true}) -- FULL OTA才需要，这里不需要
    elseif chip_target == "ec718s" or chip_target == "ec716s" then
	-- 718S/716S程序区缩小到130000，剩余134000 - 130000 = 4000 16KB空间为用户区，如果开启WIFI扫描(低速编译disable)就是剩余132000 - 130000 = 2000 8KB空间为用户区
       add_defines("AP_FLASH_LOAD_SIZE=0x130000",{public = true})
       add_defines("AP_PKGIMG_LIMIT_SIZE=0x130000",{public = true})
    end
    --加入代码和头文件
    add_includedirs("./inc",{public = true})
    add_files("./src/*.c",{public = true})

end)