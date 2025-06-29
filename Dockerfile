FROM debian:bullseye

# Installa CMake, build tools e (opzionale) dependenze
RUN apt-get update && apt-get install -y \
    wget \
    g++ \
    make \
    tar \
    && rm -rf /var/lib/apt/lists/* && \
    wget https://github.com/Kitware/CMake/releases/download/v3.29.2/cmake-3.29.2-linux-x86_64.tar.gz && \
    tar -xzf cmake-3.29.2-linux-x86_64.tar.gz && \
    mv cmake-3.29.2-linux-x86_64 /opt/cmake && \
    ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

# Scarica libtorch
RUN wget https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-2.3.0%2Bcpu.zip && \
    unzip libtorch-shared-with-deps-2.3.0+cpu.zip -d /opt && \
    rm libtorch-shared-with-deps-2.3.0+cpu.zip

# Crea directory
WORKDIR /app

# Copia tutto il progetto
COPY . .

# Crea directory di build e compila
RUN cmake -S . -B build && cmake --build build --target HAIve

# Setta working dir dove c'è il binario
WORKDIR /app/bin

# Entrypoint
CMD ["./HAIve"]
