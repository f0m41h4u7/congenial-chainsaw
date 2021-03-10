cmake_minimum_required(VERSION 3.2)

project(mq VERSION 0.0.1)

find_package(Boost 1.54.0 REQUIRED system)

add_executable(mq main.cpp)

set_target_properties(mq PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(mq
    PRIVATE "${CMAKE_BINARY_DIR}" ${Boost_INCLUDE_DIRS} /usr/local/include/rapidjson
)

target_link_libraries(mq ${CMAKE_THREAD_LIBS_INIT} boost_system pthread)

target_compile_options(mq PRIVATE
  -Wall -Wextra -pedantic
)
