FROM gcc:latest


RUN apt-get update && \
    apt-get install -y \
    git \
    gcc \
    g++ \
    cmake \
    libjsoncpp-dev \
    uuid-dev \
    openssl \
    libssl-dev \
    zlib1g-dev \
    doxygen \
    libboost-dev


COPY . /app
WORKDIR /app
RUN rm -rf build && mkdir build && cd build && cmake .. && cmake --build . --config Release --target myapp -j $(nproc) --
ENTRYPOINT [ "./build/myapp/myapp" ]
