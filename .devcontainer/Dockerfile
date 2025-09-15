FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LC_ALL=C.UTF-8
ENV USER=root

# Base packages and utilities
RUN apt update && apt install -y --no-install-recommends \
    apt-utils                       \
    bash-completion                 \
    build-essential                 \
    ca-certificates                 \
    cmake                           \
    curl                            \
    git                             \
    htop                            \
    iputils-ping                    \
    lsb-release                     \
    lsof                            \
    make                            \
    ninja-build                     \
    openssh-client                  \
    pkg-config                      \
    python3         	            \
    python3-pip        	            \
    rsync                           \
    sudo                            \
    tree                            \
    unzip                           \
    wget       	                    \
    python-is-python3               \
    vim                             \
    xz-utils                        \
    # Add user in sudoer group
    && adduser root sudo            \
    # Cleanup
    && rm -rf /var/lib/apt/lists/*

# Audio packages
RUN apt update && apt install -y --no-install-recommends \
    alsa-utils                      \
    gir1.2-gstreamer-1.0            \
    gstreamer1.0-plugins-base       \
    gstreamer1.0-plugins-good       \
    python3-gi                      \
    python3-rtmidi                  \
    python3-mido                    \
    pipewire-audio-client-libraries \
    pulseaudio-utils                \
    # Cleanup
    && rm -rf /var/lib/apt/lists/*

# C++ libraries
RUN apt update && apt install -y --no-install-recommends \
    libasound2-dev                      \
    libboost-all-dev                    \
    libgstreamer1.0-dev                 \
    libgstreamer-plugins-base1.0-dev    \
    libpulse-dev                        \
    # Cleanup
    && rm -rf /var/lib/apt/lists/*

# Dev tools
RUN apt update && apt install -y --no-install-recommends \
    clang-format                    \
    clang-tidy                      \
    gdb-multiarch                   \
    lcov                            \
    qemu-user-static                \
    valgrind                        \
    # Cleanup
    && rm -rf /var/lib/apt/lists/*

# Create the user
ARG USERNAME=user
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN userdel --remove ubuntu \
    && groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && usermod -s /bin/bash $USERNAME \
    # Add sudo support
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/localuser \
    && chmod 0440 /etc/sudoers.d/localuser

# Set the default user
USER $USERNAME
ENV USER=$USERNAME
