cmake_minimum_required(VERSION 3.2)

project(project VERSION 0.0.1)

find_package(Boost 1.54.0 REQUIRED system)

add_executable(project main.cpp)

set_target_properties(project PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(project
    PRIVATE "${CMAKE_BINARY_DIR}" ${Boost_INCLUDE_DIRS}
)

target_link_libraries(project ${CMAKE_THREAD_LIBS_INIT} boost_system pthread)

target_compile_options(project PRIVATE
  -Wall -Wextra -pedantic
)
