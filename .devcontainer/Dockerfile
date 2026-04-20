FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

RUN apt-get update -y 
RUN export DEBIAN_FRONTEND=noninteractive

RUN apt-get install -y --no-install-recommends \
    build-essential \
    python3-pip \
    libxkbcommon-x11-0 \
    libxcb-cursor0 \
    cmake \
    qtchooser \
    qt6-tools-dev-tools \
    libgl1-mesa-dev \
    libx11-xcb-dev \
    libxcb-keysyms1 \
    libxcb-keysyms1-dev \
    libxcb-image0 \
    libxcb-image0-dev \
    libxcb-shm0-dev \
    libxcb-icccm4-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-shape0-dev \
    libxcb-randr0-dev \
    libfontconfig1 \
    libxcb-render-util0-dev \
    libxcb-xinerama0-dev \
    libxkbcommon-dev \
    libdbus-1-3 \
    libxkbcommon-x11-dev \
    libicu-dev \
    usbutils

RUN pip3 install aqtinstall

RUN mkdir -p /opt/Qt

RUN pip3 install --upgrade pip && pip3 install aqtinstall
RUN mkdir -p /opt/Qt && python3 -m aqt install-qt linux desktop 6.8.2 linux_gcc_64 --outputdir /opt/Qt --modules qtserialport

ENV QT_PATH=/opt/Qt/6.8.2/gcc_64
ENV PATH=$QT_PATH/bin:$PATH
ENV LD_LIBRARY_PATH=$QT_PATH/lib
ENV QT_QPA_PLATFORM_PLUGIN_PATH=$QT_PATH/plugins

RUN apt-get install -y software-properties-common
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update -y
RUN apt-get install -y --no-install-recommends \
    gcc-13 \
    g++-13

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
RUN update-alternatives --config gcc
RUN update-alternatives --config g++

RUN apt-get clean && rm -rf /var/lib/apt/lists/*