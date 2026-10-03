# egos-2000 development image: RISC-V toolchain, QEMU, X11 libraries.
#
# Build:  docker build -t egos-env .
# Run:    .\run.ps1
#
# If github.com is unreachable, build through a proxy:
#   docker build --add-host=host.docker.internal:host-gateway \
#     --build-arg HTTPS_PROXY=http://host.docker.internal:7897 -t egos-env .

FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive

# Build tools plus the X11 client libraries SDL2 loads with dlopen().
# Without the libX* packages SDL2 falls back to its dummy driver and no window appears.
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      wget \
      ca-certificates \
      git \
      libx11-6 \
      libxext6 \
      libxcursor1 \
      libxrandr2 \
      libxi6 \
      libxinerama1 \
      libxss1 \
      libxxf86vm1 \
      x11-utils \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/egos

ENV GCC_VERSION=14.2.0-3
ENV QEMU_VERSION=8.2.2-1

# RISC-V toolchain
RUN wget -q https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/download/v${GCC_VERSION}/xpack-riscv-none-elf-gcc-${GCC_VERSION}-linux-x64.tar.gz \
 && tar -xzf xpack-riscv-none-elf-gcc-${GCC_VERSION}-linux-x64.tar.gz \
 && rm xpack-riscv-none-elf-gcc-${GCC_VERSION}-linux-x64.tar.gz

# QEMU emulator
RUN wget -q https://github.com/xpack-dev-tools/qemu-riscv-xpack/releases/download/v${QEMU_VERSION}/xpack-qemu-riscv-${QEMU_VERSION}-linux-x64.tar.gz \
 && tar -xzf xpack-qemu-riscv-${QEMU_VERSION}-linux-x64.tar.gz \
 && rm xpack-qemu-riscv-${QEMU_VERSION}-linux-x64.tar.gz

# Puts riscv-none-elf-gcc and qemu-system-riscv32 on PATH
ENV PATH="/opt/egos/xpack-riscv-none-elf-gcc-${GCC_VERSION}/bin:/opt/egos/xpack-qemu-riscv-${QEMU_VERSION}/bin:${PATH}"

# The host repository is mounted here, see the -v flag in run.ps1
WORKDIR /workspace

CMD ["/bin/bash"]
