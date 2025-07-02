FROM debian:bullseye

RUN apt-get update && apt-get install -y \
    wget \
    g++ \
    make \
    tar \
    unzip \
    && rm -rf /var/lib/apt/lists/* && \
    wget https://github.com/Kitware/CMake/releases/download/v3.29.2/cmake-3.29.2-linux-x86_64.tar.gz && \
    tar -xzf cmake-3.29.2-linux-x86_64.tar.gz && \
    mv cmake-3.29.2-linux-x86_64 /opt/cmake && \
    ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

RUN wget https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-2.3.0%2Bcpu.zip && \
    unzip libtorch-shared-with-deps-2.3.0+cpu.zip -d /app && \
    rm libtorch-shared-with-deps-2.3.0+cpu.zip && \
    mv /app/libtorch-shared-with-deps-2.3.0+cpu /app/libtorch


WORKDIR /app

COPY . .

RUN cmake -S . -B build && cmake --build build --target HAIve

WORKDIR /app/bin

CMD ["./HAIve"]
