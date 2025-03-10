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
    -- set_warnings("error")

    --加入代码和头文件
    add_includedirs("./inc",{public = true})

    -- 此demo默认添加了benchmark和widgets demo，且在lv_conf.h中已打开对应功能宏
    -- 默认演示benchmark demo
    -- 若需使用其他demo，请将下面一行代码注释，自行添加源文件，并在lv_conf.h内修打开对应功能宏
    add_files("./src/demo_benchmark.c",{public = true})
    -- 将下面注释打开并将上一行代码注释，即可演示widgets demo
    -- add_files("./src/demo_widgets.c",{public = true})

    -- 下面为外部flash font示例
    -- add_files("./src/demo_font_flash.c","./src/lvgl_flash_fonts.c")
    -- add_files("./src/bin2c.c")
    -- -- sfud
    -- -- add_defines("SFUD_USING_FAST_READ")--根据自己flash是否支持自行设置，spi频率一定要高，低速不要开启，反而会更慢
    -- add_defines("LUAT_USE_SFUD",{public = true})
    -- add_includedirs(luatos_root.."/components/sfud",{public = true})
    -- add_files(luatos_root.."/components/sfud/*.c")
	-- remove_files(luatos_root.."/components/sfud/luat_lib_sfud.c")


    add_includedirs(luatos_root.."/components/u8g2", {public = true})
    add_includedirs(luatos_root.."/components/lcd", {public = true})
    add_files(luatos_root.."/components/lcd/*.c")
    remove_files(luatos_root.."/components/lcd/luat_lib_*.c")
	add_includedirs(luatos_root.."/components/lvgl8",{public = true})
	add_includedirs(luatos_root.."/components/lvgl8/src",{public = true})
	add_includedirs(luatos_root.."/components/lvgl8/demos",{public = true})
	add_files(luatos_root.."/components/lvgl8/src/**.c",{public = true})
	add_files(luatos_root.."/components/lvgl8/demos/benchmark/**.c",{public = true})
	add_files(luatos_root.."/components/lvgl8/demos/widgets/**.c",{public = true})
end)