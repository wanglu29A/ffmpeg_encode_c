cmake_minimum_required(VERSION 3.10)
project(ffmpeg_encode_c C)

set(CMAKE_C_STANDARD 99)
link_directories(lib)
include_directories(include)
add_executable(ffmpeg_encode_c main.c)
target_link_libraries(ffmpeg_encode_c -lavcodec -lavdevice -lavfilter -lavfilter -lavformat -lavutil -lswresample -lswscale)