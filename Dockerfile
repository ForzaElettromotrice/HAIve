FROM nvidia/cuda:12.1.1-devel-ubuntu20.04

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

RUN wget wget https://download.pytorch.org/libtorch/cu121/libtorch-shared-with-deps-2.3.0%2Bcu121.zip && \
    unzip libtorch-shared-with-deps-2.3.0+cu121.zip -d /app && \
    rm libtorch-shared-with-deps-2.3.0+cu121.zip && \
    mv /app/libtorch-shared-with-deps-2.3.0+cu121 /app/libtorch


WORKDIR /app

COPY . .

RUN cmake -S . -B build && cmake --build build --target HAIve

WORKDIR /app/bin

CMD ["./HAIve"]
