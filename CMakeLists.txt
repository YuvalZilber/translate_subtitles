cmake_minimum_required(VERSION 3.23)
project(extract_subtitles_2)

set(CMAKE_CXX_STANDARD 23)

add_executable(extract_subtitles_2 src/main.cpp src/utils.cpp headers/utils.h src/font_2.cpp headers/font_2.h src/font.cpp headers/font.h)
