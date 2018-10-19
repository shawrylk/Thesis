cmake_minimum_required(VERSION 3.5.1)


project(server)


find_package(Threads)
include_directories(include)
#include_directories(/usr/local/include/)
#link_directories(/usr/local/lib/)
file(GLOB SOURCES "src/*.cpp")
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package( OpenCV REQUIRED )
add_executable(${PROJECT_NAME} ${SOURCES})
add_definitions(-D WITH_GSTREAMER=ON)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${OpenCV_LIBS}) #-lwiringPi)
