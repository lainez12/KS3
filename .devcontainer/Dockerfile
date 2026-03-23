FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

RUN apt-get update -y && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    build-essential \
    python3-pip \
    libxkbcommon-x11-0 \
    libxcb-cursor0 \
    cmake \
    qtchooser \
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

RUN apt-get clean && rm -rf /var/lib/apt/lists/*