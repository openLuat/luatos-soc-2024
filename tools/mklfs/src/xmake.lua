add_rules("mode.debug", "mode.release")

target("mklfs")
    set_kind("binary")
    add_files("lfs/*.c")
    add_files("mklfs.c")
    add_includedirs("lfs/")
