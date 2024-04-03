#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import os, struct, sys, logging, subprocess, shutil
import re

logging.basicConfig(level=logging.DEBUG)
log = logging.root

resp_headers = {}

# 原版差分文件
def diff_org(old_path, new_path, dst_path, chip="ec718p"):
    cmd = []
    if os.name != "nt":
        cmd.append("wine")
    cmd.append("FotaToolkit.exe")
    cmd.append("-d")
    if os.name != "nt":
        cmd.append("config/" + chip + ".json")
    else:
        cmd.append("config\\" + chip +".json")
    cmd.append("BINPKG")
    cmd.append(dst_path)
    cmd.append(old_path)
    cmd.append(new_path)
    subprocess.check_call(" ".join(cmd), shell=True)
    # if os.path.exists(dst_path) :
    #     with open(dst_path, "rb") as f :
    #         data = f.read()
    #         import hashlib
    #         md5 = hashlib.md5()
    #         md5.update(data)
    #         print(md5.hexdigest())

# QAT固件比较简单, 原版差分文件
def diff_qat(old_path, new_path, dst_path):
    diff_org(old_path, new_path, dst_path)

# 合宙AT固件, 就需要添加一个头部
def diff_at(old_path, new_path, dst_path):
    # 先生成一个原始差分文件
    diff_org(old_path, new_path, dst_path)
    # 然后读取数据
    with open(dst_path, "rb") as f:
        data = f.read()
    # 添加头部,生成最终文件
    with open(dst_path, "wb") as f:
        #写入头部
        head = struct.pack(">bIbI", 0x7E, 0x01, 0x7D, len(data))
        f.write(head)
        #写入原始数据
        f.write(data)

# 合宙CSDK全量升级
def diff_full(old_path, new_path, dst_path, version=None):
    import py7zr, json

    tmpdir = "soctmp"
    if os.path.exists(tmpdir) :
        shutil.rmtree(tmpdir)
    os.makedirs(tmpdir)
    def tmpp(path) :
        return os.path.join(tmpdir, path)

    
    new_param = None
    new_binpkg = None
    new_script = None
    new_mem_map = None
    new_chip_type = None
    # 首先, 解开新固件
    with py7zr.SevenZipFile(new_path, 'r') as zip:
        for fname, bio in zip.readall().items():
            if str(fname).endswith("info.json") :
                fdata = bio.read()
                new_param = json.loads(fdata.decode('utf-8'))
            if str(fname).endswith(".binpkg") :
                new_binpkg = bio.read()
            if str(fname) == "mem_map.txt" :
                new_mem_map = bio.read()
                mm = re.match(r".*#define TYPE_(\S*) 1.*", new_mem_map.decode("UTF-8"), re.S)
                if mm :
                    new_chip_type = mm.group(1)
    # 进一步解析mem_map.txt文件
    if new_mem_map == None :
        print("非法的soc文件, mem_map.txt肯定要有的")
        return False
    data = new_mem_map.decode("UTF-8")
    mm = re.match(r".*#define TYPE_(\S*) 1.*", data, re.S)
    if mm:
        chip_type = mm.group(1)
    mm = re.match(r".*#define FLASH_FS_REGION_START \(0x(\S*)\).*", data, re.S)
    if mm:
        fs_address = int(mm.group(1), 16)
    mm = re.match(r".*#define FULL_OTA_SAVE_ADDR \(0x(\S*)\).*", data, re.S)
    if mm:
        ota_save_address = int(mm.group(1), 16)
    mm = re.match(r".*#define FULL_OTA_SAVE_ADDR 0x(\S*).*", data, re.S)
    if mm:
        ota_save_address = int(mm.group(1), 16)
    mm = re.match(r".*#define CP_FLASH_LOAD_ADDR \(0x(\S*)\).*", data, re.S)
    if mm:
        cp_load_address = mm.group(1)
    mm = re.match(r".*#define AP_FLASH_LOAD_ADDR \(0x(\S*)\).*", data, re.S)
    if mm:
        ap_load_address = mm.group(1)
    mm = re.match(r".*#define AP_FLASH_XIP_ADDR \(0x(\S*)\).*", data, re.S)
    if mm:
        flash_xip_address = mm.group(1)
    
    # 逐个参数检查
    if ota_save_address == 0:
        print("没有设置FULL_OTA_SAVE_ADDR")
        return False, "没有设置FULL_OTA_SAVE_ADDR"
    
    cp_load_address = int(cp_load_address, 16) - int(flash_xip_address, 16)
    ap_load_address = int(ap_load_address, 16) - int(flash_xip_address, 16)
    app_magic = new_param['fota']['magic_num']

    # 解压出AP包
    with open("tmp.binpkg", "wb+") as f :
        f.write(new_binpkg)
    cmd = []
    if os.name != "nt":
        cmd.append("wine")
        cmd.append("dep/fcelf.exe")
    else:
        cmd.append("dep\\fcelf.exe")
    cmd.append("-E")
    cmd.append("-input")
    cmd.append("tmp.binpkg")
    cmd.append("-dir")
    cmd.append(tmpdir)
    cmd.append("-info")
    cmd.append("imagedata.json")
    print(" ".join(cmd))
    subprocess.check_call(" ".join(cmd), shell=True)
    with open("imagedata.json", encoding="UTF-8") as f :
        igd = json.load(f)
        for t in igd["imageinfo"] :
            if t["type"] == "AP" :
                with open(tmpp("aptmp.bin"), "wb+") as f :
                    with open(tmpp(t["file"]), "rb") as f2:
                        f.write(f2.read())
            if t["type"] == "CP" :
                with open(tmpp("cp.bin"), "wb+") as f :
                    with open(tmpp(t["file"]), "rb") as f2:
                        f.write(f2.read())

    # soc_tools.exe zip_file eac37218 0007e000 "_temp\combine\ap.bin" "_temp\combine\ap.zip" 40000 1
    # soc_tools.exe zip_file eac37218 0007e000 soctmp\ap.bin soctmp\ap.zip 40000 1
    # 合成AP整包
    cmd = []
    if os.name != "nt":
        cmd.append("wine")
    cmd.append("soc_tools.exe")
    cmd.append("zip_file")
    cmd.append(app_magic)
    cmd.append("{:08x}".format(ap_load_address))
    cmd.append(tmpp("aptmp.bin"))
    cmd.append(tmpp("ap.zip"))
    cmd.append("40000")
    cmd.append("1")
    print(" ".join(cmd))
    subprocess.check_call(" ".join(cmd), shell=True)

    # soc_tools.exe zip_file eac37218 0001a000 "_temp\combine\cp-demo-flash.bin" "_temp\combine\cp.zip" 40000 1
    # soc_tools.exe zip_file eac37218 0001a000 soctmp\cp.bin soctmp\cp.zip 40000 1
    # 合成CP整包
    cmd = []
    if os.name != "nt":
        cmd.append("wine")
    cmd.append("soc_tools.exe")
    cmd.append("zip_file")
    cmd.append(app_magic)
    cmd.append("{:08x}".format(cp_load_address))
    cmd.append(tmpp("cp.bin"))
    cmd.append(tmpp("cp.zip"))
    cmd.append("40000")
    cmd.append("1")
    print(" ".join(cmd))
    subprocess.check_call(" ".join(cmd), shell=True)

    bin_data = b''
    with open(tmpp("ap.zip"), "rb") as f :
        bin_data += f.read()
    with open(tmpp("cp.zip"), "rb") as f :
        bin_data += f.read()
    if len(bin_data) >= fs_address - ota_save_address:
        print("升级包超过大小限制了")
        return False, "升级包超过大小限制了"
    with open(tmpp("total.zip"), "wb") as f :
        f.write(bin_data)
    with open(tmpp("dummy.bin"), "wb") as f :
        pass

    if version == None or len(version) == 0 :
        from datetime import datetime
        dd = datetime.now()
        version = ((int(dd.year) - 2000) * 12 + int(dd.month)) * 1000000 + int(dd.day) * 10000 + int(dd.hour) * 100 + int(dd.minute)

    #cmd = "{} make_ota_file {} 4294967295 0 0 0 {} \"{}\" \"{}\" \"{}\"".format(str(soc_exe_path), app_magic, str(hex(version)), str(work_path.with_name("total.zip")), str(Path(WIN32_FILE_PATH, "ec616", 'dummy.bin')), str(Path(out_path, "{}_{}_{}.sota".format(file_name, version, file_suffix))))
    # soc_tools.exe make_ota_file eac37218 4294967295 0 0 0 0x111e7fa6 "total.zip" "dummy.bin" "out.sota"
    # soc_tools.exe make_ota_file eac37218 4294967295 0 0 0 0x111e7faa soctmp\total.zip soctmp\dummy.bin update3.bin
    cmd = []

    if os.name != "nt":
        cmd.append("wine")
    cmd.append("soc_tools.exe")
    cmd.append("make_ota_file")
    cmd.append(app_magic)
    cmd.append("4294967295")
    cmd.append("0")
    cmd.append("0")
    cmd.append("0")
    cmd.append(str(hex(version)))
    cmd.append(tmpp("total.zip"))
    cmd.append(tmpp("dummy.bin"))
    cmd.append(dst_path)
    print(" ".join(cmd))
    subprocess.check_call(" ".join(cmd), shell=True)

    print("all done")
    return True, None

# LuatOS文件的差分, CSDK也是这种,只是没有script分区
def diff_soc(old_path, new_path, dst_path):
    tmpdir = "soctmp"
    if os.path.exists(tmpdir) :
        shutil.rmtree(tmpdir)
    os.makedirs(tmpdir)
    def tmpp(path) :
        return os.path.join(tmpdir, path)
    
    import py7zr, json
    old_param = None
    old_binpkg = None
    old_mem_map = None
    old_chip_type = None

    new_param = None
    new_binpkg = None
    new_script = None
    new_mem_map = None
    new_chip_type = None
    with py7zr.SevenZipFile(old_path, 'r') as zip:
        for fname, bio in zip.readall().items():
            if str(fname).endswith("info.json") :
                fdata = bio.read()
                old_param = json.loads(fdata.decode('utf-8'))
            if str(fname).endswith(".binpkg") :
                old_binpkg = bio.read()
            if str(fname) == "mem_map.txt" :
                old_mem_map = bio.read()
                mm = re.match(r".*#define TYPE_(\S*) 1.*", old_mem_map.decode("UTF-8"), re.S)
                if mm :
                    old_chip_type = mm.group(1)
    with py7zr.SevenZipFile(new_path, 'r') as zip:
        for fname, bio in zip.readall().items():
            if str(fname).endswith("info.json") :
                fdata = bio.read()
                new_param = json.loads(fdata.decode('utf-8'))
            if str(fname).endswith(".binpkg") :
                new_binpkg = bio.read()
            if str(fname).endswith("script.bin") :
                new_script = bio.read()
            if str(fname) == "mem_map.txt" :
                new_mem_map = bio.read()
                mm = re.match(r".*#define TYPE_(\S*) 1.*", new_mem_map.decode("UTF-8"), re.S)
                if mm :
                    new_chip_type = mm.group(1)
    if not old_param or not old_binpkg:
        print("老版本不是SOC固件!!")
        return
    if not old_mem_map or not new_mem_map:
        print("没有找到mem_map.txt文件")
        return
    if old_chip_type != new_chip_type:
        print("新", old_chip_type, "新", new_chip_type, "不匹配")
        return
    script_only = new_binpkg == old_binpkg
    if script_only :
        with open("delta.par", "wb+") as f :
            pass
    else:
        with open(tmpp("old2.binpkg"), "wb+") as f :
            f.write(old_binpkg)
        with open(tmpp("new2.binpkg"), "wb+") as f :
            f.write(new_binpkg)
        diff_org(tmpp("old2.binpkg"), tmpp("new2.binpkg"), "delta.par", chip=new_chip_type)
    fstat = os.stat("delta.par")
    resp_headers["x-delta-size"] = str(fstat.st_size)
    
    if new_script :
        with open(tmpp("script.bin"), "wb+") as f :
            f.write(new_script)
        # 先对script.bin进行打包
        # cmd = "{} zip_file {} {} \"{}\" \"{}\" {} 1".format(str(soc_exe_path), 
        # new_param['fota']['magic_num'], 
        # new_param["download"]["script_addr"], 
        # str(new_path.with_name(new_param["script"]["file"])), str(new_path.with_name("script_fota.zip")), str(new_param['fota']['block_len']))
        cmd = []
        if os.name != "nt":
            cmd.append("wine")
        cmd.append("soc_tools.exe")
        cmd.append("zip_file")
        cmd.append(str(new_param['fota']['magic_num']))
        cmd.append(str(new_param["download"]["script_addr"]))
        cmd.append(tmpp("script.bin"))
        cmd.append(tmpp("script_fota.zip"))
        cmd.append(str(new_param['fota']['block_len']))
        subprocess.check_call(" ".join(cmd), shell=True)
    else:
        with open(tmpp("script_fota.zip"), "wb+") as f :
            pass

    ## 然后打包整体差分包
    # cmd = "{} make_ota_file {} 0 0 0 0 0 \"{}\" \"{}\" \"{}\"".format(str(soc_exe_path), 
    # new_param['fota']['magic_num'], 
    # str(new_path.with_name("script_fota.zip")), 
    # str(exe_path.with_name("delta.par")), 
    # str(Path(out_path, "output.sota")))
    cmd = []
    if os.name != "nt":
        cmd.append("wine")
    cmd.append("soc_tools.exe")
    cmd.append("make_ota_file")
    cmd.append(str(new_param['fota']['magic_num']))
    cmd.append("0")
    cmd.append("0")
    cmd.append("0")
    cmd.append("0")
    cmd.append("0")
    cmd.append(tmpp("script_fota.zip"))
    cmd.append("delta.par")
    cmd.append(tmpp("output.sota"))
    subprocess.check_call(" ".join(cmd), shell=True)

    shutil.copy(tmpp("output.sota"), dst_path)
    print("done soc diff")

def do_mode(mode, old_path, new_path, dst_path, is_web) :

    # 根据不同的模式执行
    if mode == "org":
        diff_org(old_path, new_path, dst_path)
    elif mode == "qat" :
        diff_qat(old_path, new_path, dst_path)
    elif mode == "at" :
        diff_at(old_path, new_path, dst_path)
    elif mode == "csdk" :
        diff_soc(old_path, new_path, dst_path)
    elif mode == "soc" :
        diff_soc(old_path, new_path, dst_path)
    elif mode == "full" :
        # TODO version 是不是要支持自定义
        diff_full(old_path, new_path, dst_path)
    else:
        print("未知模式, 未支持" + mode)
        if not is_web :
            sys.exit(1)

def start_web():
    import bottle
    from bottle import request, post, static_file, response, get
    @post("/api/diff/<mode>")
    def http_api(mode):
        if os.path.exists("diff.bin"):
            os.remove("diff.bin")
        global resp_headers
        resp_headers.clear()
        oldBinbkg = request.files.get("old")
        newBinpkg = request.files.get("new")
        if os.path.exists("old.binpkg") :
            os.remove("old.binpkg")
        if os.path.exists("new.binpkg") :
            os.remove("new.binpkg")
        oldBinbkg.save("old.binpkg")
        newBinpkg.save("new.binpkg")
        do_mode(mode, "old.binpkg", "new.binpkg", "diff.bin", True)
        return static_file("diff.bin", root=".", download="diff.bin", headers=resp_headers)
    @get("/")
    def index_page():
        return static_file("index.html", root=".")
    bottle.run(host="0.0.0.0", port=9000)

def main():
    if len(sys.argv) < 2 :
        return
    mode = sys.argv[1]
    if mode == "web" :
        start_web()
        return
    if len(sys.argv) < 5 :
        print("需要 模式 老版本路径 新版本路径 目标输出文件路径")
        print("示例:  python main.py qat old.binpkg new.binpkg diff.bin")
        print("可选模式有: at qat csdk org soc")
        sys.exit(1)
    do_mode(mode, sys.argv[2], sys.argv[3], sys.argv[4], False)

if __name__ == "__main__":
    main()
