FROM gcc:10.2.0
RUN apt-get update && apt-get install -y make git cmake libgtest-dev libboost-system-dev libboost-dev
RUN git clone https://github.com/Tencent/rapidjson && cd rapidjson && cmake . && make && make install
WORKDIR /home
COPY . .
RUN mkdir build && cd build && cmake ..
WORKDIR /home/build
ENTRYPOINT ["make"]
