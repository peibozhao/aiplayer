
FROM bitriseio/android-ndk

# minicap
RUN git clone https://github.com/DeviceFarmer/minicap /minicap
WORKDIR /minicap
RUN git submodule update --init
RUN ndk-build
RUN sed -i '/Build/{N; N; d}' run.sh

# minitouch
RUN git clone https://github.com/DeviceFarmer/minitouch /minitouch
WORKDIR /minitouch
RUN git submodule update --init
RUN ndk-build
RUN sed -i '/Build/{N; d}' run.sh

# run
WORKDIR /
COPY start.sh /
CMD bash ./start.sh; /bin/bash
