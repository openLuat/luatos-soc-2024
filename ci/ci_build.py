#!/usr/bin/python3
# -*- coding: UTF-8 -*-

'''
本脚本用于CI构建project目录下的项目, 并输出编译成功与否的统计数据

基本规则:
1. 编译项目时, 必须指定chip_target参数
2. 编译项目时, 可选指定project_root参数, 如果是单个项目, 指定project_root为项目路径且以/结尾

# 例如编译全部项目, 默认project_root是"../project"目录

python ci_build.py ec716s

# 编译指定项目
python ci_build.py ec718p ..\\project\\example_rndis\\

# 忽略指定的target
例如luatos项目只能在部分chip_target上运行, 则需要在项目根目录添加一个 ci_conf.json文件, 内容如下

{
    "build" : {
        "chip_target": ["ec718p", "ec718pv"]
    }
}

'''

import os, struct, sys, logging, subprocess, shutil
import traceback, json

logging.basicConfig(level=logging.DEBUG)
log = logging.root

# project_root = os.path.abspath(os.path.join("..","project"))

def load_project_list(chip_target, project_root):
    # 获取项目列表
    results = []
    if os.path.exists(os.path.join(project_root, "xmake.lua")):
        name = os.path.split(project_root)[-1]
        results.append({"name": name, "path": project_root})
    else:
        for name in os.listdir(project_root):
            if os.path.isdir(os.path.join(project_root, name)):
                results.append({"name":name, "path":os.path.join(project_root, name)})
    # 逐个加载项目下的ci_conf.json文件, 获取构建条件
    for project in results:
        ci_conf = os.path.join(project["path"], "ci_conf.json")
        if not os.path.exists(ci_conf):
            continue
        with open(ci_conf, encoding="utf-8") as f:
            data = json.load(f)
            # 获取构建条件
            project["build"] = data.get("build", {})
            if data.get("build", {}).get("chip_target"):
                project["build"] = {
                    "chip_target": data["build"]["chip_target"],
                }
    
    return results

def do_build(chip_target, project_root):
    # 获取项目列表
    project_list = load_project_list(chip_target, project_root)
    # 遍历项目列表, 逐个构建
    results = []
    passfail = {
        "pass" : 0,
        "fail" : 0,
        "skip" : 0,
        "unknown" : 0,
    }
    for project in project_list:
        result = do_build_project(chip_target, project)
        if result :
            results.append(result)
            if result["result"] == "pass":
                passfail["pass"] += 1
            elif result["result"] == "fail":
                passfail["fail"] += 1
            elif result["result"] == "skip":
                passfail["skip"] += 1
            else:
                passfail["unknown"] += 1
            # break
    # 对结果排序
    # 排序规则: pass > fail > skip > unknown
    results.sort(key=lambda x: (x["result"] == "pass", x["result"] == "fail", x["result"] == "skip" , x["result"] == "unknown"), reverse=True)
    # 输出结果
    log.info("==================================================")
    log.info("Build result: %s", chip_target)
    log.info("==================================================")
    for result in results:
        log.info("%s: %s", result["name"], result["result"])
    log.info("==================================================")
    log.info("Pass: %d, Fail: %d, Skip: %d", passfail["pass"], passfail["fail"], passfail["skip"])
    if 0 != passfail["fail"]:
        log.error("Build failed!")
        sys.exit(1)

def do_build_project(chip_target, project):
    log.info("==================================================")
    log.info("Build project: %s %s", chip_target, project["name"])
    log.info("==================================================")

    if "build" in project and "chip_target" in project["build"]:
        if chip_target not in project["build"]["chip_target"]:
            log.info("Skip: %s %s", chip_target, project["name"])
            return {
                "name": project["name"],
                "result": "skip",
            }
    elif project["name"] == "spinet":
        return {
                "name": project["name"],
                "result": "skip",
        }

    # 执行构建
    try :
        env = os.environ.copy()
        env["XMAKE_LOG"] = "build.log"
        env["XMAKE_ROOT"] = "y"
        subprocess.check_call(["xmake", "f", "--chip_target=%s" % chip_target, "-y"], cwd=project["path"], env=env)
        subprocess.check_call(["xmake", "clean"], cwd=project["path"], env=env)
        subprocess.check_call(["xmake", "f", "--chip_target=%s" % chip_target, "-y"], cwd=project["path"], env=env)
        subprocess.check_call(["xmake", "-y", "-P", "."], cwd=project["path"], env=env)
        # 构建成功,返回结果
        return {
            "name": project["name"],
            "result": "pass",
        }
    except:
        log.exception("Build failed!")
        return {
            "name": project["name"],
            "result": "fail",
            "reason": traceback.format_exc()
        }

def main():
    if len(sys.argv) < 2:
        log.error("Usage: %s chip_target project_root", sys.argv[0])
        return -1
    chip_target = sys.argv[1]
    global project_root
    if len(sys.argv) > 2 :
        project_root = os.path.abspath(sys.argv[2])
        log.info("使用指定项目根目录: %s", project_root)
    else:
        project_root = os.path.abspath(os.path.join("..","project"))
        log.info("使用默认项目根目录: %s", project_root)
    do_build(chip_target, project_root)

if __name__ == '__main__':
    main()