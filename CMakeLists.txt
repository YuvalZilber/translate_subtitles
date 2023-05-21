cmake_minimum_required(VERSION 3.23)
project(extract_subtitles_2)

set(CMAKE_CXX_STANDARD 23)

add_executable(extract_subtitles_2 src/main.cpp src/utils.cpp headers/utils.h src/font.cpp headers/font.h src/MkvFile.cpp headers/MkvFile.h headers/consts.h src/consts.cpp headers/base_consts.h src/base_consts.cpp src/Logger.cpp headers/Logger.h src/NullBuffer.cpp headers/NullBuffer.h src/InteractiveShell.cpp headers/InteractiveShell.h src/Vlc.cpp headers/Vlc.h src/base_utils.cpp headers/base_utils.h)
