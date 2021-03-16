FROM gcc:10.2.0
WORKDIR /home
RUN apt-get update && apt-get install -y make cmake libgtest-dev
COPY . .
RUN mkdir build && cd build && cmake ..
WORKDIR /home/build
ENTRYPOINT ["make"]
