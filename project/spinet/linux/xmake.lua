add_rules("mode.debug", "mode.release")

target("airspi")
    -- set_kind("static")
    set_optimize("fastest")
    set_warnings("extra")
    add_includedirs("inc/")
    add_cxflags("-Werror")
    add_files("src/*.c")
