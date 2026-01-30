FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential cmake git ca-certificates wget xz-utils && \
    rm -rf /var/lib/apt/lists/*

COPY . /
RUN chmod +x /entrypoint.sh

# Build application natively
RUN cmake -S / -B /build-native && \
    cmake --build /build-native -j$(nproc)

# Download and extract aarch64 toolchain from Bootlin
RUN mkdir -p /opt/toolchain && \
    wget -q https://toolchains.bootlin.com/downloads/releases/toolchains/aarch64/tarballs/aarch64--glibc--stable-2025.08-1.tar.xz -O /tmp/toolchain.tar.xz && \
    tar -xf /tmp/toolchain.tar.xz -C /opt/toolchain && \
    rm /tmp/toolchain.tar.xz

ENV PATH="${PATH}:/opt/toolchain/aarch64--glibc--stable-2025.08-1/bin"

# Cross-compile with Bootlin aarch64 toolchain
RUN cmake -S / -B /build-aarch64 -DCMAKE_TOOLCHAIN_FILE=/cmake/aarch64-toolchain.cmake && \
    cmake --build /build-aarch64 -j$(nproc)

ENTRYPOINT ["/entrypoint.sh"]
