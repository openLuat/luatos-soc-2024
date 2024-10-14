project_dir = os.scriptdir()
project_name = project_dir:match(".+[/\\]([%w_]+)")

csdk_root = "../../" --csdk根目录,可自行修改
includes(csdk_root.."csdk.lua")
description_common()

-- 此为创建的实例目标
target("hello_world",function()
    -- 此目标为一个静态库
    set_kind("static")
    -- 设置输出目录
    set_targetdir("$(buildir)/".. "hello_world" .. "/")
    -- 添加csdk配置
    description_csdk()

    --加入代码和头文件
    add_includedirs("./inc",{public = true})
    add_files("./src/hello_world.c",{public = true})

end)

target(project_name,function()
    set_kind("static")
    set_targetdir("$(buildir)/".. project_name .. "/")
    description_csdk()
    set_warnings("error")
    -- 依赖hello_world静态库
    add_deps("hello_world")
    --加入代码和头文件
    add_includedirs("./inc",{public = true})
    add_files("./src/example_main.c",{public = true})
end)
