cmake_minimum_required(VERSION 3.7)

project(archmine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

add_definitions(-Wall -g -D_DEBUG)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIRS})
link_libraries(${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})


add_executable(${PROJECT_NAME}
${CMAKE_SOURCE_DIR}/archmine.cpp
${CMAKE_SOURCE_DIR}/sound.cpp)