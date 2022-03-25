
FROM ubuntu:18.04 as basic-env
RUN apt update
RUN apt install -y ffmpeg libsdl2-2.0-0 adb wget gcc git pkg-config ninja-build libsdl2-dev libavcodec-dev libavdevice-dev libavformat-dev libavutil-dev libusb-1.0-0 libusb-1.0-0-dev
# meson installed by apt has low version
RUN apt install -y python3-pip
RUN pip3 install meson


FROM basic-env as compile-env
RUN git clone -b feature/remote https://github.com/peibozhao/scrcpy /scrcpy
WORKDIR /scrcpy
RUN apt install -y sudo
RUN ./install_release.sh


FROM basic-env as run-env
COPY --from=compile-env /usr/local/bin/scrcpy /usr/local/bin/scrcpy
COPY --from=compile-env /usr/local/share/scrcpy/scrcpy-server /usr/local/share/scrcpy/scrcpy-server
RUN mkdir /scrcpy
COPY start.sh /scrcpy
WORKDIR /scrcpy
EXPOSE 22331
EXPOSE 22332

CMD bash ./start.sh

