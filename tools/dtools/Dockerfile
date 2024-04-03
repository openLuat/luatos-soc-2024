# FROM ccr.ccs.tencentyun.com/wendal/ecfota-base:latest
FROM wendal/ecfota-base
# FROM ubuntu:22.04
# RUN sed -i 's/security.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list
# RUN sed -i 's/archive.ubuntu.com/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list
# ENV TZ=Asia/Shanghai
# ENV DEBIAN_FRONTEND=noninteractive
# COPY FotaToolkit.exe /opt/
# COPY dep/ /opt/dep/
# COPY *.dll /opt/
# COPY config /opt/config/
# #COPY *.binpkg /opt/
# WORKDIR /opt/
# RUN dpkg --add-architecture i386 &&\
#     apt-get update &&\
#     apt-get install -y wget &&\
#     apt-get install -y wine32 wine64 &&\
#     apt clean -y

# # 调用一次FotaToolkit,这样才能初始化wine环境,不然启动耗时会很长
# RUN wine FotaToolkit.exe -h || date

RUN apt update && apt install -y python3-pip && pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple py7zr && apt clean -y

COPY main.py /opt/
COPY bottle.py /opt/
COPY soc_tools.exe /opt/
COPY FotaToolkit.exe /opt/
#COPY deps   /opt/deps/
COPY dep /opt/dep/
COPY config /opt/config/
COPY index.html /opt/

EXPOSE 9000

CMD [ "/usr/bin/python3", "main.py", "web"]
