cmake_minimum_required(VERSION 3.15)
project(ultrasound_live)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS "-framework GLUT -framework OpenGL")

add_library(ultrasound_live_lib SHARED
        main.cpp
        main.h)

add_executable(ultrasound_live main.cpp)

target_link_libraries(ultrasound_live
        ${GLUT_LIBRARIES}
        ${GLU_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ultrasound_live_lib)