# 差分包生成工具

本文件夹是生成差分包所需要的工具, 分2部分:
1. windows原生工具
2. 可运行原生工具的docker镜像

支持的差分功能:
1. CSDK生产的binpkg差分,功能名称csdk
2. 合宙系列AT固件的差分,功能名称at
3. QAT系列固件的差分,功能名称qat
4. LuatOS固件的soc文件之间的差分,功能名称soc
5. 原始SDK的binpkg差分,功能名称org
6. CSDK全量更新

## 原生工具的用法

### 通过脚本跑

如果是LuatOS的soc文件差分,需要先安装依赖项

```
pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple py7zr
```

差分命令格式及示例

```
python3 main.py 模式 老版本固件路径 新版本固件路径 输出差分包的路径

python3 main.py csdk old.soc new.soc diff.bin
```

其中
* 模式, 可选值有`csdk` `at` `qat` `org` `soc` `full`
* AT固件传binpkg路径, CSDK和LuatOS固件传soc文件路径
* 对于全量升级`full`, 老版本固件可以使用空文件替代,但必须传

## Docker镜像说明

鉴于服务器大多是linux系统,而fota工具又没有linux系统, 这里提供docker镜像

构建镜像
```
docker build -t wendal/ecfota7xx .
```

运行镜像, 暴露9000端口, 会创建一个http api
```
docker run -it --rm -p 9000:9000 wendal/ecfota7xx
```

### http api调用规则

```
URL         /api/diff/<mode>
METHOD      POST
使用文件上传的方式 老文件的参数名 old, 新文件的参数名 new
响应结果是diff.bin
<mode> 是模式, 与命令行的模式一致
```

注意: docker镜像的web服务也是 `main.py` 提供的,非必须, 其他编程语言也可以直接在镜像内调用差分工具

## 其他信息

未尽事宜,请咨询FAE或销售
