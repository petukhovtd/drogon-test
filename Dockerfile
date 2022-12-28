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
RUN cd build && rm -r ./* && cmake .. && cmake --build . --config Debug --target all -j 10 --
CMD [ "./build/drogon-test" ] 
EXPOSE 3000
