FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LC_ALL=C.UTF-8
ENV USER=root

RUN apt update && apt install -y --no-install-recommends \
    alsa-utils                      \
    apt-utils                       \
    bash-completion                 \
    build-essential                 \
    ca-certificates                 \
    clang-format                    \
    clang-tidy                      \
    cmake                           \
    curl                            \
    git                             \
    gdb-multiarch                   \
    iputils-ping                    \
    lsb-release                     \
    make                            \
    openssh-client                  \
    pipewire-audio-client-libraries \
    pulseaudio-utils                \
    python3         	            \
    python3-pip        	            \
    qemu-user-static                \
    rsync                           \
    sudo                            \
    unzip                           \
    wget       	                    \
    python-is-python3               \
    xz-utils                        \
    # Add user in sudoer group
    && adduser root sudo            \
    # Cleanup
    && rm -rf /var/lib/apt/lists/*

# # Install arm toolchain
# ARG ARM_TOOLCHAIN_PKG=arm-gnu-toolchain-11.3.rel1-x86_64-aarch64-none-linux-gnu
# ARG ARM_TOOLCHAIN_URL=https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/${ARM_TOOLCHAIN_PKG}.tar.xz
# RUN wget ${ARM_TOOLCHAIN_URL} \
#     && tar xJf ${ARM_TOOLCHAIN_PKG}.tar.xz -C /opt

# # Define the toolchain base path
# ENV ARM_TOOLCHAIN_PATH=/opt/${ARM_TOOLCHAIN_PKG}

# # AArch64 (Cross-Compilation)
# ENV CROSS_COMPILE_AARCH64=${ARM_TOOLCHAIN_PATH}/bin/aarch64-none-linux-gnu
# ENV CC_AARCH64=${CROSS_COMPILE_AARCH64}-gcc
# ENV CXX_AARCH64=${CROSS_COMPILE_AARCH64}-g++
# ENV AR_AARCH64=${CROSS_COMPILE_AARCH64}-ar

# # Set environment for QEMU
# ENV AARCH64_SYSROOT=${ARM_TOOLCHAIN_PATH}/aarch64-none-linux-gnu/libc
# RUN sudo ln -s ${AARCH64_SYSROOT}/lib/ld-linux-aarch64.so.1 /lib/ld-linux-aarch64.so.1

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
