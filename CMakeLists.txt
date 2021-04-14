cmake_minimum_required(VERSION 3.7)

project(archmine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

add_definitions(-Wall -g -D_DEBUG)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-2.4) 


include_directories(${GTKMM_INCLUDE_DIRS})
link_libraries(${GTKMM_LIBRARIES})

add_executable(${PROJECT_NAME}
${CMAKE_SOURCE_DIR}/archmine.cpp
${CMAKE_SOURCE_DIR}/rtns.cpp
${CMAKE_SOURCE_DIR}/dialogs.cpp
${CMAKE_SOURCE_DIR}/multiplayer.cpp
${CMAKE_SOURCE_DIR}/graphics.cpp)

find_package(X11 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE X11::X11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)