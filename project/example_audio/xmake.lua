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
    --加入amr编解码库
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_common/dec/include",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_nb/common/include",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_nb/dec/include",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_wb/dec/include",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/opencore-amrnb",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/opencore-amrwb",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/oscl",{public = true})
    add_includedirs(luatos_root .. "/components/multimedia/amr_decode/amr_nb/enc/src",{public = true})
    -- **.c会递归所有子文件夹下的文件
    add_files(luatos_root .. "/components/multimedia/amr_decode/**.c",{public = true})
	add_files(luatos_root.."/components/multimedia/luat_audio_es8311.c")
	add_files(luatos_root.."/components/multimedia/luat_audio_tm8211.c")
    add_linkdirs(csdk_root.."/lib",csdk_root.."/PLAT/core/lib",{public = true})
    -- -- 此处使用 libaisound50_16K.a, 还可选择 8K版本:libaisound50_8K.a,8K英文版本:libaisound50_8K_eng.a,16K英文版本:libaisound50_16K_eng.a
    add_linkgroups("mp3","tts_res","aisound50_16K", {whole = true,public = true})

end)